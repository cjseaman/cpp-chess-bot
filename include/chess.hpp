#ifndef CHESS_HPP
#define CHESS_HPP

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>

#define U64 uint64_t

using namespace std;

extern U64 piece_vision[7][64];
extern U64 piece_bnb[7][64];
extern U64 behind[64][64];

void generate_piece_vision_maps();
int bit_scan_forward(U64 bb);
U64 squares_behind(int p1, int p2);
void generate_behind();
U64 blocked_piece_vision(int p, U64 occupied, SquareOccupant piece_type);
int count_ones(U64 b);
void generate_blocker_permutations(uint64_t mask, vector<U64>& blocker_permutations);
int posi(U64 position);
void check_piece_vision();
void print_bitboard(U64 board);
U64 east_one(U64 board);
U64 west_one(U64 board);
U64 north_one(U64 board);
U64 south_one(U64 board);
U64 draw_king_vision(U64 start_position);
U64 draw_rook_rays(U64 start_position);
U64 get_rook_attacks(U64 start_position, U64 occupancy);
U64 draw_knight_vision(U64 start_position);
U64 draw_bishop_rays(U64 start_position);

enum Color {
    LIGHT = 1, DARK = -1
};

enum SquareOccupant {
    EMPTY, KING, PAWN, ROOK, KNIGHT, BISHOP, QUEEN
};

enum CardinalDirection {
    NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST, ALL
};

enum MoveType {
    PUSH, DOUBLE_PUSH, ENPASSANT, CAPTURE, QUIET, CASTLE, PROMOTION, CAPTURE_AND_PROMOTION
};

struct Init {
    Init() {
        generate_piece_vision_maps();
        generate_behind();
    }
};

struct Move {
    int from;
    int to;
    SquareOccupant piece;
    int captured;
    int promotion;
    MoveType type;
};

static U64 FILE_A = 0x0101010101010101ULL;
static U64 FILE_B = 0x0202020202020202ULL;
static U64 FILE_G = 0x4040404040404040ULL;
static U64 FILE_H = 0x8080808080808080ULL;
static U64 RANK_1 = 0x00000000000000FFULL;
static U64 RANK_2 = 0x000000000000FF00ULL;
static U64 RANK_7 = 0x00FF000000000000ULL;
static U64 RANK_8 = 0xFF00000000000000ULL;

struct Piece {
    SquareOccupant type;
    int square_number;
    Color color;
};

class Game {
    public:
        Piece board[64];
        vector<Piece> light_pieces;
        vector<Piece> dark_pieces;
    Game(string start_position_fen);
};

std::vector<U64> position_in_each_square();

#endif

