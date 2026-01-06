Layer 2 Ethernet switch implemented in userland under Linux, based on AF_PACKET sockets.
It dynamically learns MAC addresses, is VLAN-aware (802.1Q), and performs forwarding/flooding like a traditional switch.
Can be monitor through a web page.

✨ Features

    📡 Capture Ethernet frames via AF_PACKET/SOCK_RAW
    🧠 MAC learning table with aging
    🏷️ 802.1Q VLAN support
    🌊 VLAN-aware flooding
    🔁 Unicast forwarding
    📊 RX/TX statistics per interface (frames + bytes)
    ⏱️ Uptime and last_seen per MAC entry
    🖥️ Real-time web interface for monitoring
<img width="1920" height="306" alt="image" src="https://github.com/user-attachments/assets/d96611f1-49e5-4c05-881d-5f3646ade894" />

<img width="1048" height="532" alt="image" src="https://github.com/user-attachments/assets/4bebf4b2-b44e-4b1d-80a8-45166f927fdb" />
Processes diagram and API

🛠️ Install & Build
```
git clone git@github.com:bornvj/Vswitch.git
cd Vswitch
make
```

▶️ Run
```
sudo ./switch
```

