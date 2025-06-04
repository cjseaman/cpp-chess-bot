#include "../include/chess.hpp"
#include <chrono>

void check_piece_vision() {
    U64 center = 1ULL << 27;

    U64 n_center = 1ULL << 59;
    U64 e_center = 1ULL << 31;
    U64 s_center = 1ULL << 3;
    U64 w_center = 1ULL << 24;

    U64 ne_corner = 1ULL << 63;
    U64 se_corner = 1ULL << 7;
    U64 sw_corner = 1ULL;
    U64 nw_corner = 1ULL << 56;

    vector<PieceType> pieces_to_test = {EMPTY, ROOK, KNIGHT, BISHOP, KING, QUEEN};

    for(PieceType piece : pieces_to_test) {
        cout << "Testing " << piece << endl;
        print_bitboard(piece_vision[piece][posi(center)]);
        print_bitboard(piece_vision[piece][posi(n_center)]);
        print_bitboard(piece_vision[piece][posi(e_center)]);
        print_bitboard(piece_vision[piece][posi(s_center)]);
        print_bitboard(piece_vision[piece][posi(w_center)]);
        print_bitboard(piece_vision[piece][posi(ne_corner)]);
        print_bitboard(piece_vision[piece][posi(se_corner)]);
        print_bitboard(piece_vision[piece][posi(sw_corner)]);
        print_bitboard(piece_vision[piece][posi(nw_corner)]);
    }
}

void time_perft_test(string position, int depth) {
    cout << "Testing at depth = " << depth << endl;
    GameState state = parse_full_fen(position);
    GameNode head = {state};
    auto start = std::chrono::high_resolution_clock::now();
    MoveCounter results = build_perft_move_tree(state, depth, head);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    cout << "Completed in " << duration.count() / 1000000 << " seconds\n";
    cout << results;
}

void do_all_perft_tests(int depth) {
    cout << "Testing All Perft at depth = " << depth << endl;
    vector<string> perft_fen_strings = {starting_fen, position_2, position_3, position_4};
    vector<vector<MoveCounter>> perft_results = {starting_perft_results, position_2_perft_results, position_3_perft_results, position_4_perft_results};
    GameState state;
    

    for(int i = 0; i < perft_fen_strings.size(); i++) {
        state = parse_full_fen(perft_fen_strings[i]);
        for(int j = 0; j <= depth; j++) {
            GameNode head = {state};
            cout << "Testing Position " << i + 1 << " Perft(" << j << "): ";
            auto start = std::chrono::high_resolution_clock::now();
            MoveCounter results = build_perft_move_tree(state, j, head);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            if(results == perft_results[i][j]) {
                cout << "Passed in " << duration.count() << " microseconds" << "\n";
            } else {
                cout << "\nFailed. Expecting:\n" << perft_results[i][j] << "\n Results:\n" << results << '\n';
            }
        }
    }
}


