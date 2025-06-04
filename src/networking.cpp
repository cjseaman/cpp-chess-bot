#include "../include/chess.hpp"
#include "../include/httplib.h"

void serve_best_move(string host, int port) {
    // HTTP
    httplib::Server svr;

    svr.Get("/best_move", [](const httplib::Request& req, httplib::Response &res) {
        string fen_string = "";
        int depth = 0;
        if(req.has_param("fen")) {
            fen_string = req.get_param_value("fen");
        } else {
            res.set_content("No fen specified", "text/plain");
            return;
        }
        if(req.has_param("depth")) {
            depth = std::stoi(req.get_param_value("depth"));
        } else {
            res.set_content("No depth specified", "text/plain");
        }

        MoveEval best_move;
        GameState state = parse_full_fen(fen_string);
        best_move = alpha_beta_search(state, depth);
        string content = square_to_algebraic(best_move.best_move.from) + square_to_algebraic(best_move.best_move.to);
        res.set_content(content, "text/plain");
    });

    svr.listen(host, port);
}