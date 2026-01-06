#define _GNU_SOURCE

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <linux/if_packet.h>
#include <linux/if_link.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <signal.h>

#include "interface.h"
#include "frame.h"
#include "record.h"
#include "tools.h"
#include "command.h"

#include "switch_ctx.h"

volatile sig_atomic_t running = 1;
pid_t web_pid = -1;


#define BUFFERSIZE 1600
#define OUTPUT_BUFFERSIZE 1600


void handle_signal(int sig)
{
    if (sig == SIGINT)
        printf("Ctrl+C pressed\n");
    else if (sig == SIGTERM)
        printf("Termination signal received\n");

    running = 0;
    if (web_pid > 0)
        kill(web_pid, SIGTERM);
}

int main(void)
{
    // Preparation for the signal handling
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    unlink("/run/vswitch.sock");

    // Init the switch context
    switch_ctx ctx;
    memset(&ctx, 0, MAX_IFACES * sizeof(struct iface));
    ctx.nbr_ifaces = 0;

    
    // split processes
    web_pid = fork();
    if (web_pid == 0) // WEB SERVER
    {
        if (freopen("/tmp/web.log", "w", stdout) == NULL || freopen("/tmp/web.log", "w", stderr) == NULL)
        {
            perror("freopen");
            exit(1);
        }
        if (execl("/bin/sh", "sh", "./run.sh", NULL))
        {
            perror("execve");
            exit(1);
        }
    }
    else // SWITCH
    {
        // SOCKET UNIX ================
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, "/run/vswitch.sock");

        int sockfd_un = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (bind(sockfd_un, (struct sockaddr*)(&addr), sizeof(sa_family_t) + strlen(addr.sun_path) + 1)) // formula to take the size of the string in count
        {
            perror("bind");
            exit(EXIT_FAILURE);
        }
        chmod("/run/vswitch.sock", 0666); // make it readable for flask
        // ============================

        struct ifaddrs *ifaddr;

        if (getifaddrs(&ifaddr) == -1) 
        {
            perror("getifaddrs");
            exit(EXIT_FAILURE);
        }

        for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
        {
            if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_PACKET)
                continue;


            printf("%-8s\n", ifa->ifa_name);

            int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
            if (sock < 0) 
            {
                perror("socket");
                exit(EXIT_FAILURE);
            }

            struct sockaddr_ll addr;
            memset(&addr, 0, sizeof(addr));
            addr.sll_family   = AF_PACKET;
            addr.sll_protocol = htons(ETH_P_ALL);
            addr.sll_ifindex  = ((struct sockaddr_ll *) ifa->ifa_addr)->sll_ifindex;

            if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
            {
                perror("bind");
                exit(1);
            }
            
            ctx.ifaces[ctx.nbr_ifaces].ifindex      = addr.sll_ifindex;
            ctx.ifaces[ctx.nbr_ifaces].sock         = sock;
            ctx.ifaces[ctx.nbr_ifaces].addr         = addr;
            ctx.ifaces[ctx.nbr_ifaces].ifname       = strdup(ifa->ifa_name);

            ctx.ifaces[ctx.nbr_ifaces].rx_frames    = 0;
            ctx.ifaces[ctx.nbr_ifaces].rx_bytes     = 0;
            ctx.ifaces[ctx.nbr_ifaces].tx_frames    = 0;
            ctx.ifaces[ctx.nbr_ifaces].tx_bytes     = 0;

            ctx.nbr_ifaces++;
        }

        freeifaddrs(ifaddr);

        ctx.start_time = time(NULL);

        unsigned char buf[BUFFERSIZE];
        char outputbuff[BUFFERSIZE];

        while (running)
        {
            fd_set readfds;
            FD_ZERO(&readfds);

            int maxfd = 0;
            for (size_t i = 0; i < ctx.nbr_ifaces; i++) 
            {
                FD_SET(ctx.ifaces[i].sock, &readfds);
                if (ctx.ifaces[i].sock > maxfd)
                    maxfd = ctx.ifaces[i].sock;
            }

            FD_SET(sockfd_un, &readfds);
            if (sockfd_un > maxfd)
                maxfd = sockfd_un;

            if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0)
            {
                if (!running)
                    break;
                perror("select");
                break;
            }
            
            // When the unix_socket is awake
            if (FD_ISSET(sockfd_un, &readfds))
            {
                struct sockaddr_un src_addr;
                socklen_t addrlen = sizeof(src_addr);

                ssize_t len = recvfrom(sockfd_un, buf, sizeof(buf), 0, (struct sockaddr *)&src_addr, &addrlen);
                
                if (len >= 0 && len < BUFFERSIZE)
                    buf[len] = '\0';


                printf("UNIX -> [");
                for (ssize_t i = 0; i < len; i++)
                    putchar(buf[i]);
                printf("]\n");
                
                command *cmd = parseCommand(buf, len);

                printCommand(cmd);

                if (!cmd)
                    continue;


                memset(outputbuff, 0, sizeof(outputbuff));
                handleCommand(cmd, outputbuff, ctx);

                sendto(sockfd_un, outputbuff, strlen(outputbuff), 0, (struct sockaddr *)&src_addr, addrlen);
            
                free(cmd);
            }

            // find the awake interface
            for (size_t in = 0; in < ctx.nbr_ifaces; in++)
            {
                if (!FD_ISSET(ctx.ifaces[in].sock, &readfds))
                    continue;

                struct sockaddr_ll src_addr;
                socklen_t addrlen = sizeof(src_addr);

                ssize_t len = recvfrom(ctx.ifaces[in].sock, buf, sizeof(buf), 0, (struct sockaddr *)&src_addr, &addrlen);
                if (len <= 0)
                    continue;

                frame *f = parseFrame(buf, len);

                if (!f)
                    continue;

                ctx.ifaces[in].rx_frames += 1;
                ctx.ifaces[in].rx_bytes += len;

                // learn
                mac_table_learn(f->src, f->vlan_id, ctx.ifaces[in].ifindex);

                // lookup for destination
                record *dst = mac_table_lookup(f->dst, f->vlan_id);

                if (dst && dst->INTERFACE != ctx.ifaces[in].ifindex)
                {
                    int out = dst->INTERFACE;

                    sendto(ctx.ifaces[out].sock, buf, len, 0, (struct sockaddr *)&ctx.ifaces[out].addr,
                        sizeof(ctx.ifaces[out].addr));

                    ctx.ifaces[out].tx_frames += 1;
                    ctx.ifaces[out].tx_bytes += len;
                }
                else
                {
                    for (size_t out = 0; out < ctx.nbr_ifaces; out++)
                    {
                        if (ctx.ifaces[out].ifindex == ctx.ifaces[in].ifindex)
                            continue;

                        sendto(ctx.ifaces[out].sock, buf, len, 0, (struct sockaddr *)&ctx.ifaces[out].addr,
                            sizeof(ctx.ifaces[out].addr));

                        ctx.ifaces[out].tx_frames += 1;
                        ctx.ifaces[out].tx_bytes += len;
                    }
                }
                free(f);
            }
            mac_table_age(time(NULL));

            // UNCOMMENT THIS LINE TO PRINT MAC TABLE
            // print_mac_table(ctx.ifaces, ctx.nbr_ifaces, ctx.start_time);
        }

        if (web_pid > 0)
            waitpid(web_pid, NULL, 0);

        unlink("/run/vswitch.sock");

        exit(EXIT_SUCCESS);
    }
}