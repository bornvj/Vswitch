async function refresh() {
    const res = await fetch("/api/ifaces");
    const data = await res.json();
    document.getElementById("output").textContent = data.raw;
}

setInterval(refresh, 3  * 1000);
refresh();