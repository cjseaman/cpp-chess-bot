#ifndef CHESS_HPP
#define CHESS_HPP

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <cstdint>
#include <sstream>
#include <algorithm>

#define U64 uint64_t

using std::string;
using std::vector;
using std::list;
using std::cout;
using std::endl;
using std::istringstream;
using std::invalid_argument;
using std::remove_if;

extern U64 piece_vision[7][64];
extern U64 piece_bnb[7][64];
extern U64 not_behind[64][64];

enum Color {
    WHITE = 1, BLACK = -1, NONE = 0
};

enum ColorOccupancyIndexes {
    B_OCC, E_OCC, W_OCC
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

enum CastlingRights {
    WHITE_KINGSIDE  = 1 << 0, // 0001
    WHITE_QUEENSIDE = 1 << 1, // 0010
    BLACK_KINGSIDE  = 1 << 2, // 0100
    BLACK_QUEENSIDE = 1 << 3  // 1000
};

static PieceType legal_promotions[4] = {ROOK, KNIGHT, BISHOP, QUEEN};

void generate_piece_vision_maps();
void generate_not_behind();

struct Init {
    Init() {
        generate_piece_vision_maps();
        generate_not_behind();
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

struct UnMakeInfo {
    uint8_t castling_rights;
    int en_passant_square;
    int halfmove_clock;
};

struct Piece {
    PieceType type;
    int square_number;
    Color color;
};

struct Occupancy {
    U64 colors[3] = {};
    U64 all = 0;
};

struct GameState {
    vector<Piece> pieces;
    Occupancy occupancy;
    Color side_to_move;
    uint8_t castle_rights;
    int en_passant_square;  // -1 if none
    int halfmove_clock;
    int fullmove_number;
};

struct MoveCounter {
    U64 move_count = 0;
    U64 capture_count = 0;
    U64 en_passant_count = 0;
    U64 castle_count = 0;
    U64 promotion_count = 0;
    U64 check_count = 0;
    U64 checkmate_count = 0;
};

struct GameNode {
    GameState state;
    Move move = {-1, -1, EMPTY, EMPTY, EMPTY, QUIET};
    vector<GameNode> children;
    MoveCounter move_counter;
};

int bit_scan_forward(U64 bb);
U64 squares_behind(int p1, int p2);
U64 get_piece_attack_vision(int p, U64 occupied, PieceType piece_type);
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
vector<Move> get_all_pseudo_legal_moves(GameState& state);
Occupancy get_occupancy(vector<Piece>& pieces);
int algebraic_to_square(const string& alg);
void make_move(GameState& state, Move& move);
void handle_move(GameState& state, int pi, int from, int to);
int prune_illegal_moves(GameState& state, vector<Move>& legal_moves);
bool is_illegal_move(GameState& state, Move& last_move);
U64 get_enemy_attack_vision(GameState& state, Color color);
MoveCounter build_move_tree(GameState& state, int depth, GameNode& node);

std::ostream& operator<<(std::ostream& os, const GameState& state);
std::ostream& operator<<(std::ostream& os, const Move& move);
std::ostream& operator<<(std::ostream& os, const MoveCounter& counter);

static U64 FILE_A = 0x0101010101010101ULL;
static U64 FILE_B = 0x0202020202020202ULL;
static U64 FILE_G = 0x4040404040404040ULL;
static U64 FILE_H = 0x8080808080808080ULL;
static U64 RANK_1 = 0x00000000000000FFULL;
static U64 RANK_2 = 0x000000000000FF00ULL;
static U64 RANK_7 = 0x00FF000000000000ULL;
static U64 RANK_8 = 0xFF00000000000000ULL;
static U64 WK_GAP = 0x0000000000000060ULL;
static U64 WQ_GAP = 0x000000000000000EULL;
static U64 BK_GAP = 0x6000000000000000ULL;
static U64 BQ_GAP = 0x0E00000000000000ULL;

#endif

