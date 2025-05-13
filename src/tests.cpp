#include "../include/chess.hpp"

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

    vector<SquareOccupant> pieces_to_test = {EMPTY, ROOK, KNIGHT, BISHOP, KING, QUEEN};

    for(SquareOccupant piece : pieces_to_test) {
        cout << "Testing " << piece << endl;
        print_bitboard(piece_vision[piece][posi(center)]);
        cout << piece_vision[piece][posi(center)] << endl;
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
