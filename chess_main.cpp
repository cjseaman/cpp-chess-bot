#include "include/chess.hpp"
#include <thread>

int main(int argc, char** argv) {
    std::string line;
    GameState state;
    bool stop_search = false;
   
    while (1) {
        std::getline(std::cin, line);
        std::stringstream ss(line);
        string token = "";
        vector<string> tokens;
        while(ss >> token) {
            tokens.push_back(token);
        }
        int i = 0;
        string this_token;
        this_token = tokens[0];
        while(i < this_token.size()) {
            if(is_valid_token(this_token)) {
                token = this_token;
                break;
            }
            i++;
            this_token = tokens[i];
        }
        if (token == "uci") {
            cout << "id name CollinBot\n";
            cout << "id author Collin\n";
            cout << "uciok\n";
        }
        else if (token == "isready") {
            cout << "readyok\n";
        }
        else if (token == "ucinewgame") {
            state = parse_full_fen(starting_fen);
        }
        else if (token == "position") {
            string position = tokens[++i];
            if(position == "startpos") {
                state = parse_full_fen(starting_fen);
            } else {
                // Take full fen string
                while(token != "moves" && i < tokens.size()) {
                    i++;
                    token = tokens[i];
                    position += token;
                    state = parse_full_fen(position);
                }
            }
            if(++i < tokens.size()) {
                token = tokens[i];
            }
            if(token == "moves") {
                while(++i < tokens.size()) {
                    Move move = verbose_to_move(tokens[i], state.pieces);
                    make_move(state, move);
                }
            }
        }
        else if (token == "go") {
            int depth = 3;
            if(tokens[++i] == "depth") {
                depth = stoi(tokens[++i]);
            }
            cout << "info depth " << depth << endl;
            MoveEval best_move = alpha_beta_search(state, depth, stop_search);
            cout << "info score " << best_move.evaluation << endl;
            cout << "bestmove " << move_to_verbose(best_move.best_move) << endl;
        }
        else if (token == "stop") {
            stop_search = true;
        }
        else if (line == "quit") {
            break;
        }
    }

    return 0;
}