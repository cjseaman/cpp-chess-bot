#ifndef CHESS_HPP
#define CHESS_HPP

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <cstdint>
#include <sstream>
#include <algorithm>
#include <cfloat>
#include <chrono>
#include <thread>

#define U64 uint64_t

using std::string;
using std::vector;
using std::list;
using std::cout;
using std::endl;
using std::istringstream;
using std::invalid_argument;
using std::remove_if;
using std::chrono::high_resolution_clock;
using std::chrono::system_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

extern U64 piece_vision[7][64];
extern U64 piece_bnb[7][64];
extern U64 not_behind[64][64];

enum Color {
    WHITE = 1, BLACK = -1, NONE = 0
};

enum CASTLE_SQUARES {
    ROOK_WK = 7, ROOK_WQ = 0, ROOK_BK = 63, ROOK_BQ = 56,
    CASTLE_WK = 5, CASTLE_WQ = 3, CASTLE_BK = 61, CASTLE_BQ = 59,
    WHITE_KING_SQ = 4, BLACK_KING_SQ = 60, WK_SHORT_SQ = 6, WK_LONG_SQ = 2, BK_SHORT_SQ = 62, BK_LONG_SQ = 58
};

enum ColorOccupancyIndexes {
    B_OCC, E_OCC, W_OCC
};

static float SPACE_WEIGHT_SCALAR = 0;
static float CENTRALITY[64] = 
{
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.0,
    0.0, 0.1, 0.2, 0.2, 0.2, 0.2, 0.1, 0.0,
    0.0, 0.1, 0.2, 0.3, 0.3, 0.2, 0.1, 0.0,
    0.0, 0.1, 0.2, 0.3, 0.3, 0.2, 0.1, 0.0,
    0.0, 0.1, 0.2, 0.2, 0.2, 0.2, 0.1, 0.0,
    0.0, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0                          
};

static float KING_SAFETY[64] = 
{
    0.3, 0.2, 0.1, 0.0, 0.0, 0.1, 0.2, 0.3,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.3, 0.2, 0.1, 0.0, 0.0, 0.1, 0.2, 0.3  
};

static float DIAGONALITY[64] = 
{
    0.2, 0.1, 0.0, 0.0, 0.0, 0.0, 0.1, 0.2,
    0.1, 0.2, 0.2, 0.1, 0.1, 0.2, 0.2, 0.1,
    0.0, 0.2, 0.3, 0.2, 0.2, 0.3, 0.2, 0.0,
    0.0, 0.1, 0.2, 0.3, 0.3, 0.2, 0.1, 0.0,
    0.0, 0.1, 0.2, 0.3, 0.3, 0.2, 0.1, 0.0,
    0.0, 0.2, 0.3, 0.2, 0.2, 0.3, 0.2, 0.0,
    0.1, 0.2, 0.2, 0.1, 0.1, 0.2, 0.2, 0.1,
    0.2, 0.1, 0.0, 0.0, 0.0, 0.0, 0.1, 0.2                          
};

static float PIECE_VALUES[7] = {
    0,     3.5,   1,    4,    3,      3.5,    9
};

enum PieceType {
    EMPTY, KING, PAWN, ROOK, KNIGHT, BISHOP, QUEEN
};

static float MATERIAL_TOTAL = PIECE_VALUES[KING] + PIECE_VALUES[PAWN] * 8 + PIECE_VALUES[ROOK] * 2 + PIECE_VALUES[KNIGHT] * 2 + PIECE_VALUES[BISHOP] * 2 + PIECE_VALUES[QUEEN];
static int MATERIAL_COUNT = 32;

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

struct MoveEval {
    Move best_move;
    float evaluation;
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
vector<Move> get_pawn_attacks(GameState& state, Piece& piece);
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
void make_move(GameState& state, const Move& move);
void handle_move(GameState& state, int pi, int from, int to);
int prune_illegal_moves(GameState& state, vector<Move>& legal_moves);
bool is_illegal_move(GameState& state, Move& last_move);
U64 get_enemy_attack_vision(GameState& state, Color color);
MoveCounter build_perft_move_tree(GameState& state, int depth, GameNode& node);
bool is_king_in_check(GameState& state, Color color, int king_square = -1, U64 enemy_attack_vision = 0xFFFFFFFFFFFFFFFFULL);

std::ostream& operator<<(std::ostream& os, const GameState& state);
std::ostream& operator<<(std::ostream& os, const Move& move);
std::ostream& operator<<(std::ostream& os, const MoveCounter& counter);
int operator==(const MoveCounter& lhs, const MoveCounter& rhs);
void do_all_perft_tests(int depth);
void time_perft_test(string position, int depth);
float evaluate(GameState& state);
MoveEval negamax_search(GameState& state, int depth);
float negamax(GameState& state, int depth);
MoveEval alpha_beta_search(GameState& state, int depth, bool& stop);
vector<Move> get_all_legal_moves(GameState& state);
void serve_best_move(string host, int port);
bool is_valid_token(string token);
string move_to_verbose(Move& move);
Move verbose_to_move(const string& verbose, vector<Piece>& pieces);
float calculate_piece_value(Piece& piece);
MoveEval ab_iterative_deepener(GameState& state, bool& stop, int max_time_ms);


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

static string starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
static string position_2 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
static string position_3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 ";
static string position_4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
static string position_5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";

static vector<MoveCounter> starting_perft_results = {{1}, {20}, {400}, {8902, 34, 0, 0, 0, 12}, {197281, 1576, 0, 0, 0, 469, 8}, {4865609, 82719, 258, 0, 0, 27351, 347}};
static vector<MoveCounter> position_2_perft_results = {{1}, {48, 8, 0, 2}, {2039, 351, 1, 91, 0, 3}, {97862, 17102, 45, 3162, 0, 993, 1}, {4085603, 757163, 1929, 128013, 15172, 25523, 43}, {193690690, 35043416, 73365, 4993637, 8392, 3309887, 30171}};
static vector<MoveCounter> position_3_perft_results = {{1}, {14, 1, 0, 0, 0, 2}, {191, 14, 0, 0, 0, 10}, {2812, 209, 2, 0, 0, 267}, {43238, 3348, 123, 0, 0, 1680, 17}, {674624, 52051, 1165, 0, 0, 52950}};
static vector<MoveCounter> position_4_perft_results = {{1, 0, 0, 0, 0, 1}, {6}, {264, 87, 0, 6, 48, 10}, {9467, 1021, 4, 0, 120, 38, 22}, {422333, 131393, 0, 7795, 60032, 15492, 5}, {15833292, 2046173, 6512, 0, 329464, 200568, 50562}};

#endif

