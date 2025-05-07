#include "../include/chess.hpp"

using namespace std;

#define U64 uint64_t

U64 east_one(U64 board) {
    return (board & ~FILE_H) << 1;
}
U64 west_one(U64 board) {
    return (board & ~FILE_A) >> 1;
}
U64 north_one(U64 board) {
    return board << 8;
}
U64 south_one(U64 board) {
    return board >> 8;
}

// Piece class declarations. Pieces have vision which tells the move process where they may go according to the rules. This accounts for out of bounds but does not account for collision or checks. 

King::King(U64 init_position, bool init_can_castle, Color init_color) {
    position = init_position;
    can_castle = init_can_castle;
    piece_color = init_color;
}

Pawn::Pawn(U64 init_position, bool init_en_passant, Color init_color) {
    position = init_position;
    en_passant = init_en_passant;
    piece_color = init_color;
}

Rook::Rook(U64 init_position, bool init_can_castle, Color init_color) {
    position = init_position;
    can_castle = init_can_castle;
    piece_color = init_color;
}

U64 King::king_vision() {
    U64 vision = east_one(position) | west_one(position);
    U64 horizontal_line = vision | position;
    vision = vision | north_one(horizontal_line) | south_one(horizontal_line);
    return vision;
}

U64 Pawn::p_attack_vision() {
    U64 vision;
    if (piece_color == LIGHT) {
        vision = east_one(north_one(position)) | west_one(north_one(position));
    } 
    if (piece_color == DARK) {
        vision = east_one(south_one(position)) | west_one(south_one(position));
    }
    return vision;
}
U64 Pawn::p_move_vision() {
    U64 vision;
    if(piece_color == LIGHT) {
        vision = north_one(position);
        if(position & RANK_2) {
            vision = north_one(vision) | north_one(position);
        }
    }
    if(piece_color == DARK) {
        vision = south_one(position);
        if(position & RANK_7) {
            vision = south_one(vision) | south_one(position);
        }
    }
    return vision;
}

U64 Rook::rook_vision() {
    U64 vision;
    //TODO
    return vision;
}

std::vector<U64> position_in_each_square() {
    U64 position;
    U64 final_position = 0;
    std::vector<U64> all_positions;
    for(position = 1; position != final_position; position = position << 1) {
        all_positions.push_back(position);
    }
    return all_positions;
}

void print_bitboard(U64 board) {
    for (int row = 7; row >= 0; --row) {
        for (int col = 0; col < 8; ++col) {
            int index = row * 8 + col;
            cout << ((board >> index) & 1ULL);
            cout << " ";
        }
        cout << "\n";
    }
} 

 