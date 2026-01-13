const ifaceState = {};

async function initIfaces() {
    try {
        const res = await fetch("/api/ifaces");
        const data = await res.json();

        const container = document.getElementById("ifaces");
        container.innerHTML = "";

        for (const ifaceName of data.ifaces.sort()) {
            ifaceState[ifaceName] = {
                open: false,
                detailsDiv: null,
                ifaceDiv: null
            };

            // Interface line
            const iface = document.createElement("div");
            iface.className = "iface";
            iface.innerHTML = `
                <span>${ifaceName}</span>
                <span class="mac">▶</span>
            `;

            // Details block
            const details = document.createElement("div");
            details.className = "details";
            details.textContent = "loading...";

            ifaceState[ifaceName].detailsDiv = details;
            ifaceState[ifaceName].ifaceDiv = iface;

            iface.addEventListener("click", () => {
                const state = ifaceState[ifaceName];
                state.open = !state.open;

                iface.classList.toggle("open", state.open);
                iface.querySelector(".mac").textContent = state.open ? "▼" : "▶";
                details.style.display = state.open ? "block" : "none";
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

            state.detailsDiv.innerHTML = "";

            // Stats
            const stats = document.createElement("div");
            stats.className = "stat";
            stats.innerHTML = `
                <div>RX frames</div><div>${got_iface.rx_frames}</div>
                <div>RX bytes</div><div>${got_iface.rx_bytes}</div>
                <div>TX frames</div><div>${got_iface.tx_frames}</div>
                <div>TX bytes</div><div>${got_iface.tx_bytes}</div>
            `;
            state.detailsDiv.appendChild(stats);

            // MAC table
            got_iface.mac
                .sort((a, b) => a.last_seen - b.last_seen)
                .forEach(mac => {
                    const line = document.createElement("div");
                    line.className = "mac";
                    line.textContent = `[${mac.address}] last seen: ${mac.last_seen}s`;
                    state.detailsDiv.appendChild(line);
                });
        }
    } catch (err) {
        console.error("Failed to refresh data", err);
    }
}

// Refresh loop
setInterval(refreshTraffic, 500);

// Init
initIfaces();