from flask import Flask, jsonify, render_template, send_from_directory
import socket
import json
import os

SOCK_PATH = "/run/vswitch.sock"
DATAGRAM_MAX_SIZE = 65535

app = Flask(__name__, static_folder='static')

def query_switch(cmd: str) -> str:
    s = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)

    client_path = "/tmp/vswitch_client.sock"
    try:
        os.unlink(client_path)
    except FileNotFoundError:
        pass

    s.bind(client_path)

    s.sendto(cmd.encode(), SOCK_PATH)
    data, _ = s.recvfrom(DATAGRAM_MAX_SIZE)

    s.close()
    os.unlink(client_path)

    return data.decode()

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/favicon.ico")
def favicon():
    return send_from_directory(
        os.path.join(app.root_path, "static"),
        "favicon.ico",
        mimetype="image/vnd.microsoft.icon"
    )

@app.route("/api/ifaces")
def ifaces():
    raw = query_switch("GET IFACES")
    # Convertit la chaîne JSON du C en objet Python
    data = json.loads(raw)
    return jsonify({"ifaces": data})

if __name__ == "__main__":
    app.run(host="127.0.0.1", port=8080, debug=True)