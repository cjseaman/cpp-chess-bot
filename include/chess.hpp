#ifndef CHESS_HPP
#define CHESS_HPP

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>

#define U64 uint64_t

using namespace std;

enum Color {
    LIGHT = 1, DARK = -1
};

enum SquareOccupant {
    EMPTY, KING, PAWN, ROOK, KNIGHT, BISHOP, QUEEN
};

enum CardinalDirection {
    NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST
};

static uint64_t FILE_A = 0x0101010101010101ULL;
static uint64_t FILE_H = 0x8080808080808080ULL;
static uint64_t RANK_2 = 0x000000000000FF00ULL;
static uint64_t RANK_7 = 0x00FF000000000000ULL;

static int mailbox[120] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7, -1,
    -1,  8,  9, 10, 11, 12, 13, 14, 15, -1,
    -1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
    -1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
    -1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
    -1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
    -1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
    -1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static int mailbox64[64] = {
    21, 22, 23, 24, 25, 26, 27, 28,
    31, 32, 33, 34, 35, 36, 37, 38,
    41, 42, 43, 44, 45, 46, 47, 48,
    51, 52, 53, 54, 55, 56, 57, 58,
    61, 62, 63, 64, 65, 66, 67, 68,
    71, 72, 73, 74, 75, 76, 77, 78,
    81, 82, 83, 84, 85, 86, 87, 88,
    91, 92, 93, 94, 95, 96, 97, 98
};

class Piece {
    public:
        U64 position;
        Color piece_color;
        U64 trace_ray(U64 start_position, uint8_t ray_length, CardinalDirection ray_direction);
};

class King : Piece {
    public: 
        bool can_castle;
        U64 king_vision();
        King(U64 init_position, bool init_can_castle, Color init_color);
};

class Pawn : Piece {
    public:
        bool en_passant;
        U64 p_attack_vision();
        U64 p_move_vision();
        Pawn(U64 init_position, bool init_en_passant, Color init_color);
};

class Rook : Piece {
    public:
        U64 rook_vision();
        bool can_castle;
    Rook(U64 init_position, bool init_can_castle, Color init_color);
};

class Knight : Piece {
    public:
        U64 knight_vision();
};

class Bishop : Piece {
    public:
        U64 bishop_vision();
};

class Queen : Bishop, Rook {
    public:
        U64 queen_vision();
};

class Board {
    public:
        SquareOccupant square_matrix[8][8];
        void set_position(string fen_starting_position);
};

class Game {
    public:
        Board board;
        vector<Piece> pieces;
};

std::vector<U64> position_in_each_square();

void print_bitboard(U64 board);

U64 east_one(U64 board);

U64 west_one(U64 board);

U64 north_one(U64 board);

U64 south_one(U64 board);

#endif

