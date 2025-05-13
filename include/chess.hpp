#ifndef CHESS_HPP
#define CHESS_HPP

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <sstream>

#define U64 uint64_t

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::istringstream;
using std::invalid_argument;

extern U64 piece_vision[7][64];
extern U64 piece_bnb[7][64];
extern U64 behind[64][64];

enum Color {
    WHITE = 1, BLACK = -1
};

enum PieceType {
    EMPTY, KING, PAWN, ROOK, KNIGHT, BISHOP, QUEEN
};

enum CardinalDirection {
    NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST, ALL
};

enum MoveType {
    PUSH, DOUBLE_PUSH, ENPASSANT, CAPTURE, QUIET, CASTLE, PROMOTION, CAPTURE_AND_PROMOTION
};

static PieceType legal_promotions[4] = {ROOK, KNIGHT, BISHOP, QUEEN};

void generate_piece_vision_maps();
void generate_behind();

struct Init {
    Init() {
        generate_piece_vision_maps();
        generate_behind();
    }
};

struct Move {
    int from;
    int to;
    PieceType piece;
    PieceType captured;
    PieceType promotion;
    MoveType type;
};

struct Piece {
    PieceType type;
    int square_number;
    Color color;
};

struct Occupancy {
    U64 white = 0;
    U64 black = 0;
    U64 all = 0;
    U64 not_all = 0;
};

struct GameState {
    vector<Piece> pieces;
    Color side_to_move;
    bool white_can_castle_kingside;
    bool white_can_castle_queenside;
    bool black_can_castle_kingside;
    bool black_can_castle_queenside;
    int en_passant_square;  // -1 if none
    int halfmove_clock;
    int fullmove_number;
};


int bit_scan_forward(U64 bb);
U64 squares_behind(int p1, int p2);
U64 get_piece_vision(int p, U64 occupied, PieceType piece_type);
U64 get_pawn_move_vision(int square_number, Color color);
U64 get_pawn_attack_vision(int square_number, Color color);
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
void print_board_array(Piece board[64]);
GameState parse_full_fen(string fen);
void print_game_state(GameState state);
string square_to_algebraic(int square);
vector<Move> get_all_legal_moves(GameState& state);
Occupancy get_occupancy(vector<Piece>& pieces);

std::ostream& operator<<(std::ostream& os, const GameState& state);

static U64 FILE_A = 0x0101010101010101ULL;
static U64 FILE_B = 0x0202020202020202ULL;
static U64 FILE_G = 0x4040404040404040ULL;
static U64 FILE_H = 0x8080808080808080ULL;
static U64 RANK_1 = 0x00000000000000FFULL;
static U64 RANK_2 = 0x000000000000FF00ULL;
static U64 RANK_7 = 0x00FF000000000000ULL;
static U64 RANK_8 = 0xFF00000000000000ULL;

#endif

