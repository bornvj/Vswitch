// ifaceState[ifname] = { open: bool, detailsDiv: HTMLElement }
const ifaceState = {};

async function initIfaces() {
    try {
        const res = await fetch("/api/ifaces");
        const data = await res.json();

        const container = document.getElementById("ifaces");
        container.innerHTML = "";

        for (const ifaceName of data.ifaces) {
            ifaceState[ifaceName] = { open: false, detailsDiv: null };

            // iface block
            const iface = document.createElement("div");
            iface.className = "iface";
            iface.textContent = ifaceName;

            // detail block
            const details = document.createElement("div");
            details.className = "details";
            details.style.display = "none";
            details.textContent = "loading...";

            ifaceState[ifaceName].detailsDiv = details;

            // Toggle open / close
            iface.addEventListener("click", () => {
                const state = ifaceState[ifaceName];
                state.open = !state.open;
                details.style.display = state.open ? "block" : "none";

                // Refresh immédiat à l’ouverture
                if (state.open) {
                    refreshTraffic(ifaceName);
                }
            });

            container.appendChild(iface);
            container.appendChild(details);
        }
    } catch (err) {
        console.error("Failed to init interfaces", err);
    }
}

async function refreshTraffic(ifaceName) {
    try {
        const res = await fetch(`/api/trafic/${ifaceName}`);
        const data = await res.json();

        const details = ifaceState[ifaceName].detailsDiv;
        if (!details) return;

        details.textContent =
            `Interface: ${ifaceName}
            RX frames: ${data.rx_frames}
            RX bytes:   ${data.rx_bytes}
            TX frames: ${data.tx_frames}
            TX bytes:   ${data.tx_bytes}`;
    } catch (err) {
        console.error(`Failed to refresh traffic for ${ifaceName}`, err);
    }
}


setInterval(() => {
    for (const ifaceName in ifaceState) {
        if (ifaceState[ifaceName].open) {
            refreshTraffic(ifaceName);
        }
    }
}, 3000);

initIfaces();