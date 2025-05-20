#include "include/chess.hpp"

int main() {
    cout << "Testing" << endl;
    string starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    string mate_in_1 = "4r2k/1p3rbp/2p1N1p1/p3n3/P2NB1nq/1P6/4R1P1/B1Q2RK1 b - - 4 32";
    string mate_in_3 = "7r/4r1k1/6P1/p2R3K/2P2P1P/1Q4q1/1P6/3R4 w - - 1 4";
    string pawn_on_7th = "8/4P3/8/8/8/8/8/8 w - - 0 1";
    string pawn_test_w2p = "8/2P2p2/8/8/8/3p3P/1p2P3/8 w - - 0 1";
    string pawn_test_b2p = "8/2P2p2/8/8/8/3p3P/1p2P3/8 b - - 0 1";
    string ep_test = "4k3/8/8/3Pp3/3P4/8/2P5/4K3 w - e6 0 1";
    string castling_test_w2p = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
    string castling_test_b2p = "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1";
    string castle_and_pawn_mess = "4k3/1P4P1/8/3Pp3/3P4/N1Q4P/P1P5/R3K2R w K e6 0 1";
    string position_1 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    string position_5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    string target_fen = "4k3/8/2q5/3Pp3/8/8/8/7K w - e6 0 1";

    GameState state = parse_full_fen(starting_fen);
    cout << state;
    cout << "\n" << "Perft Testing:\n";
    GameNode head = {state};
    int depth = 4;
    for(int i = depth; i > 0; i--) {
        cout << "Perft(" << i << ") Nodes: " << build_move_tree(state, i, head) << "\n";
    }

    cout << endl;
    return 1;
}