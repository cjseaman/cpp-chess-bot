#include "../include/chess.hpp"
#include "../include/httplib.h"

void serve_best_move(string host, int port) {
    // HTTP
    httplib::Server svr;
    

    svr.Get("/best_move", [](const httplib::Request& req, httplib::Response &res) {
        string fen_string = "";
        int depth = 0;
        bool stop = false;
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
        vector<Move> moves = get_all_legal_moves(state);

        if(is_king_in_check(state, state.side_to_move) && moves.size() == 0) {
            int win = -state.side_to_move;
            res.set_content("Checkmate " + std::to_string(win), "text/plain");
            return;
        }
        if(moves.size() == 0) {
            res.set_content("Stalemate 0", "text/plain");
            return;
        }

        best_move = alpha_beta_search(state, depth, stop);
        string content = square_to_algebraic(best_move.best_move.from) + "-" + square_to_algebraic(best_move.best_move.to) + " " + std::to_string(best_move.evaluation);
        res.set_content(content, "text/plain");
    });

    svr.listen(host, port);
}