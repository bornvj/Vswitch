from flask import Flask, jsonify, render_template
import socket

SOCK_PATH = "/run/vswitch.sock"
DATAGRAM_MAX_SIZE = 65535

app = Flask(__name__)

def query_switch(cmd: str) -> str:
    s = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
    s.sendto(cmd.encode(), SOCK_PATH)
    data, _ = s.recvfrom(DATAGRAM_MAX_SIZE)
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