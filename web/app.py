from flask import Flask, jsonify, render_template
import socket
import os

SOCK_PATH = "/run/vswitch.sock"
DATAGRAM_MAX_SIZE = 65535

app = Flask(__name__)

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

@app.route("/api/ifaces")
def stats():
    raw = query_switch("GET IFACES")
    return jsonify({"raw": raw})

if __name__ == "__main__":
    app.run(host="127.0.0.1", port=8080, debug=True)