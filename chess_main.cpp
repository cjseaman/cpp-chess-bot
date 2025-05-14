#include "include/chess.hpp"

int main() {
    cout << "Testing" << endl;
    string starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    string mate_in_1 = "4r2k/1p3rbp/2p1N1p1/p3n3/P2NB1nq/1P6/4R1P1/B1Q2RK1 b - - 4 32";
    string mate_in_3 = "7r/4r1k1/6P1/p2R3K/2P2P1P/1Q4q1/1P6/3R4 w - - 1 4";
    string pawn_on_7th = "8/4P3/8/8/8/8/8/8 w - - 0 1";
    GameState state = parse_full_fen(pawn_on_7th);
    cout << state;
    vector<Move> legal_moves = get_all_legal_moves(state);

    for(Move move : legal_moves) {
        cout << move;
    }
    
    cout << endl;
}