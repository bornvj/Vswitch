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

                // refresh at opening
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

async function refreshTraffic() {
    try {
        const res = await fetch("/api/data");
        const data = await res.json();

        for (const got_iface of data.ifaces) {
            const state = ifaceState[got_iface.name];
            if (!state || !state.open) continue;

            state.detailsDiv.innerHTML =
                `Interface: ${got_iface.name} <br/>
                RX frames: ${got_iface.rx_frames}
                RX bytes:  ${got_iface.rx_bytes}
                TX frames: ${got_iface.tx_frames}
                TX bytes:  ${got_iface.tx_bytes} <br/>` + 'test';

            console.log(got_iface);
        }

    } catch (err) {
        console.error("Failed to refresh data", err);
    }
}


setInterval(() => {
    refreshTraffic()
}, 3000);

initIfaces();