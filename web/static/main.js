async function refresh() {
    const res = await fetch("/api/ifaces");
    const data = await res.json();

    const container = document.getElementById("ifaces");
    container.innerHTML = "";

    for (ifaceName of data["ifaces"])
    {
        // Iface holder
        const iface = document.createElement("div");
        iface.className = "iface";
        iface.textContent = ifaceName;

        // Iface detail holder
        const details = document.createElement("div");
        details.className = "details";
        details.textContent = `Details for ${ifaceName} (placeholder)`;

        // Toggle listener for iface detail
        iface.addEventListener("click", () => {
            details.style.display =
                details.style.display === "none" ? "block" : "none";
        });

        container.appendChild(iface);
        container.appendChild(details);
    };
}

refresh()