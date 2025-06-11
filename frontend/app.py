from flask import Flask, render_template, request
import requests

app = Flask(__name__)

CHESS_HOST = "localhost"
CHESS_PORT = 8000
DEFAULT_DEPTH = 3

@app.route("/")
def home():
    return render_template("chessboard.html")

@app.route("/best_move", methods=["GET"])
def best_move():
    if "fen" in request.args:
        fen_string = request.args["fen"]
    else:
        return {"Error": "No fen specified!"}
    if "depth" in request.args:
        depth = request.args["depth"]
    else:
        depth = DEFAULT_DEPTH
    payload = {"fen": fen_string, "depth": depth}

    response = requests.get(f"http://{CHESS_HOST}:{CHESS_PORT}/best_move", params=payload)
    print("RESPONSE", response.url, response.content)
    move = response.text.split(" ")[0]
    balance = response.text.split(" ")[1]

    return {"move": move, "balance": balance}
