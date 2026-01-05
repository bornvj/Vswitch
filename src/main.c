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

volatile sig_atomic_t running = 1;
pid_t web_pid = -1;


#define BUFFERSIZE 1600
#define MAX_IFACES 20

struct iface ifaces[MAX_IFACES];

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

        size_t nbr_ifaces = 0;

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
            
            ifaces[nbr_ifaces].ifindex      = addr.sll_ifindex;
            ifaces[nbr_ifaces].sock         = sock;
            ifaces[nbr_ifaces].addr         = addr;
            ifaces[nbr_ifaces].ifname       = strdup(ifa->ifa_name);

            ifaces[nbr_ifaces].rx_frames    = 0;
            ifaces[nbr_ifaces].rx_bytes     = 0;
            ifaces[nbr_ifaces].tx_frames    = 0;
            ifaces[nbr_ifaces].tx_bytes     = 0;

            nbr_ifaces++;
        }

        freeifaddrs(ifaddr);
        time_t start_time = time(NULL);
        unsigned char buf[BUFFERSIZE];

        while (running)
        {
            fd_set readfds;
            FD_ZERO(&readfds);

            int maxfd = 0;
            for (size_t i = 0; i < nbr_ifaces; i++) 
            {
                FD_SET(ifaces[i].sock, &readfds);
                if (ifaces[i].sock > maxfd)
                    maxfd = ifaces[i].sock;
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
                struct sockaddr_ll src_addr;
                socklen_t addrlen = sizeof(src_addr);

                ssize_t len = recvfrom(sockfd_un, buf, sizeof(buf), 0, (struct sockaddr *)&src_addr, &addrlen);
                printf("UNIX -> [");
                for (ssize_t i = 0; i < len; i++)
                    putchar(buf[i]);
                printf("]\n");
                
                // TODO :
            }

            // find the awaik interface
            for (size_t in = 0; in < nbr_ifaces; in++)
            {
                if (!FD_ISSET(ifaces[in].sock, &readfds))
                    continue;

                struct sockaddr_ll src_addr;
                socklen_t addrlen = sizeof(src_addr);

                ssize_t len = recvfrom(ifaces[in].sock, buf, sizeof(buf), 0, (struct sockaddr *)&src_addr, &addrlen);
                if (len <= 0)
                    continue;

                frame *f = parseFrame(buf, len);

                if (!f)
                    continue;

                ifaces[in].rx_frames += 1;
                ifaces[in].rx_bytes += len;

                // learn
                mac_table_learn(f->src, f->vlan_id, ifaces[in].ifindex);

                // lookup for destination
                record *dst = mac_table_lookup(f->dst, f->vlan_id);

                if (dst && dst->INTERFACE != ifaces[in].ifindex)
                {
                    int out = dst->INTERFACE;

                    sendto(ifaces[out].sock, buf, len, 0, (struct sockaddr *)&ifaces[out].addr,
                        sizeof(ifaces[out].addr));

                    ifaces[out].tx_frames += 1;
                    ifaces[out].tx_bytes += len;
                }
                else
                {
                    for (size_t out = 0; out < nbr_ifaces; out++)
                    {
                        if (ifaces[out].ifindex == ifaces[in].ifindex)
                            continue;

                        sendto(ifaces[out].sock, buf, len, 0, (struct sockaddr *)&ifaces[out].addr,
                            sizeof(ifaces[out].addr));

                        ifaces[out].tx_frames += 1;
                        ifaces[out].tx_bytes += len;
                    }
                }
                free(f);
            }
            mac_table_age(time(NULL));

            // UNCOMMENT THIS LINE TO PRINT MAC TABLE
            // print_mac_table(ifaces, nbr_ifaces, start_time);
        }

        if (web_pid > 0)
            waitpid(web_pid, NULL, 0);

        unlink("/run/vswitch.sock");

        exit(EXIT_SUCCESS);
    }
}