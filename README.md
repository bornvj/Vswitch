switch Ethernet Layer 2 implémenté en userland sous Linux, basé sur des sockets AF_PACKET.
Il apprend dynamiquement les adresses MAC, est VLAN-aware (802.1Q) et effectue le forwarding / flooding comme un switch classique.

✨ Features

    📡 Capture des trames Ethernet via AF_PACKET / SOCK_RAW
    🧠 MAC learning table avec aging
    🏷️ Support VLAN 802.1Q
    🌊 Flooding VLAN-aw are
    🔁 Forwarding unicast
    📊 Statistiques RX / TX par interface (frames + bytes)
    ⏱️ Uptime et last_seen par entrée MAC
    🖥️ Affichage temps réel

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

