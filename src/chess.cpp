#include "../include/chess.hpp"

#define U64 uint64_t

U64 piece_vision[7][64];
U64 piece_bnb[7][64];
U64 not_behind[64][64];

U64 bishop_attacks[64][512];
U64 rook_attacks[64][4096];

static Init initializer; 

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

int posi(U64 position) {
    if((position & (position - 1)) == 0) {
        return __builtin_ctzll(position);
    } else {
        throw invalid_argument("Position must have only one bit set to 1 (must be power of 2)");
        return -1;
    }
}

int count_ones(U64 b) {
    return __builtin_popcountll(b);
}

int count_zeros(U64 b) {
    return __builtin_ctzll(b);
}

int bit_scan_forward(U64 bb) {
    return bb ? __builtin_ctzll(bb) : -1;

}
U64 squares_behind(int p1, int p2) {
    // AI Generated
    const int directions[8] = { 1, -1, 8, -8, 9, -9, 7, -7 };  // E, W, N, S, NE, SW, NW, SE
    U64 result = 0;

    for (int dir : directions) {
        int sq = p1;
        while (true) {
            sq += dir;

            if (sq < 0 || sq >= 64) break;

            int fileDiff = abs((sq % 8) - (p1 % 8));
            int rankDiff = abs((sq / 8) - (p1 / 8));
            if ((dir == 1 || dir == -1) && rankDiff != 0) break;
            if ((dir == 8 || dir == -8) && fileDiff != 0) break;
            if ((dir == 9 || dir == -9 || dir == 7 || dir == -7) &&
                fileDiff != rankDiff) break;

            if (sq == p2) {
                while (true) {
                    sq += dir;
                    if (sq < 0 || sq >= 64) break;
                    fileDiff = abs((sq % 8) - (p1 % 8));
                    rankDiff = abs((sq / 8) - (p1 / 8));
                    if ((dir == 1 || dir == -1) && rankDiff != 0) break;
                    if ((dir == 8 || dir == -8) && fileDiff != 0) break;
                    if ((dir == 9 || dir == -9 || dir == 7 || dir == -7) &&
                        fileDiff != rankDiff) break;

                    result |= 1ULL << sq;
                }
                return result;
            }
        }
    }

    return 0ULL;
}

void generate_not_behind() {
    for(int i = 0; i < 64; i++) {
        for(int j = 0; j < 64; j++) {
            not_behind[i][j] = ~squares_behind(i, j);
        }
    }
}

void generate_piece_vision_maps() {
    for(int i = 0; i < 64; i++) {
        U64 position = 1ULL << i;

        U64 rook_rays = draw_rook_rays(position);
        U64 bishop_rays = draw_bishop_rays(position);
        U64 outer_squares = FILE_A | FILE_H | RANK_1 | RANK_8;

        piece_bnb[EMPTY][i] = position;
        piece_bnb[KING][i] = 0;
        piece_bnb[ROOK][i] = rook_rays & ~outer_squares;
        piece_bnb[BISHOP][i] = bishop_rays & ~outer_squares;
        piece_bnb[KNIGHT][i] = 0;
        piece_bnb[QUEEN][i] = piece_bnb[BISHOP][i] | piece_bnb[ROOK][i];

        piece_vision[EMPTY][i] = position;
        piece_vision[KING][i] = draw_king_vision(position);
        piece_vision[ROOK][i] = rook_rays;
        piece_vision[KNIGHT][i] = draw_knight_vision(position);
        piece_vision[BISHOP][i] = bishop_rays;
        piece_vision[QUEEN][i] = piece_vision[BISHOP][i] | piece_vision[ROOK][i];
    }
}

U64 draw_king_vision(U64 start_position) {
    U64 vision = east_one(start_position) | west_one(start_position);
    U64 horizontal_line = vision | start_position;
    vision = vision | north_one(horizontal_line) | south_one(horizontal_line);
    return vision;
}

U64 draw_rook_rays(U64 start_position) {
    U64 west_rays = 0;
    U64 east_rays = 0;
    U64 north_rays = 0;
    U64 south_rays = 0;
    int i;
    // Fill left
    if(!(start_position & FILE_A)) {
        while(!(west_rays & FILE_A)) {
            west_rays = west_rays | start_position;
            west_rays = west_rays >> 1;
        }
    }
    // Fill right
    if(!(start_position & FILE_H)) {
        while(!(east_rays & FILE_H)) {
            east_rays = east_rays | start_position;
            east_rays = east_rays << 1;
        }
    }
    // Fill top and bottom
    for(i = 0; i < 7; i++) {
        north_rays = north_rays | start_position;
        north_rays = north_rays << 8;

        south_rays = south_rays | start_position;
        south_rays = south_rays >> 8;
    }
    
    return west_rays | east_rays | north_rays | south_rays;
}

U64 draw_knight_vision(U64 start_position) {
    U64 vision = 0;
    vision = vision | ((start_position << 17) & ~FILE_A);
    vision = vision | ((start_position << 10) & ~(FILE_A | FILE_B));
    vision = vision | ((start_position >> 6) & ~(FILE_A | FILE_B));
    vision = vision | ((start_position >> 15) & ~FILE_A);

    vision = vision | ((start_position << 15) & ~FILE_H);
    vision = vision | ((start_position << 6) & ~(FILE_G | FILE_H));
    vision = vision | ((start_position >> 10) & ~(FILE_G | FILE_H));
    vision = vision | ((start_position >> 17) & ~FILE_H);

    return vision;
}

U64 draw_bishop_rays(U64 start_position) {
    U64 northeast_ray = 0;
    U64 southeast_ray = 0;
    U64 northwest_ray = 0;
    U64 southwest_ray = 0;
    int i;

    // Fill West
    if(!(start_position & FILE_A)) {
        
        if(!(start_position & RANK_1)) {
            i = 1;
            while(!(southwest_ray & (FILE_A | RANK_1))) {
                southwest_ray = southwest_ray | start_position >> i >> i*8;
                i++;
            }
        }
        if(!(start_position & RANK_8)) {
            i = 1;
            while(!(northwest_ray & (FILE_A | RANK_8))) {
                northwest_ray = northwest_ray | start_position >> i << i*8;
                i++;
            }
        }
    }
    // Fill East
    if(!(start_position & FILE_H)) {
        if(!(start_position & RANK_1)) {
            i = 1;
            while(!(southeast_ray & (FILE_H | RANK_1))) {
                southeast_ray = southeast_ray | start_position << i >> i*8;
                i++;
            }
        }
        if(!(start_position & RANK_8)) {
            i = 1;
            while(!(northeast_ray & (FILE_H | RANK_8))) {
                northeast_ray = northeast_ray | start_position << i << i*8;
                i++;
            }
        }
    }
    return northeast_ray | southeast_ray | northwest_ray | southwest_ray;
}

U64 get_pawn_attack_vision(int p, Color color) {
    int attack_right_square = p + 9 * color;
    int attack_left_square = p + 7 * color;
    U64 vision = (1ULL << attack_right_square) | (1ULL << attack_left_square);
    return vision;
}

U64 get_piece_attack_vision(int p, U64 occupied, PieceType piece_type) {

    U64 vision = piece_vision[piece_type][p];
    if(piece_type == ROOK || piece_type == BISHOP || piece_type == QUEEN) {
        for(U64 b = occupied & piece_vision[piece_type][p]; b != 0; b = b & (b - 1)) {
            int sq = bit_scan_forward(b);
            U64 s_not_behind = not_behind[p][sq];
            vision = vision & not_behind[p][sq];
        }
    }
    return vision;
}

Piece fen_to_piece(char piece_char, int square_number) {
    piece_char = tolower(piece_char);
    Color color = isupper(piece_char) ? WHITE : BLACK;
    PieceType type;
    switch(piece_char) {
        case 'r':
            type = ROOK;
            break;
        case 'n':
            type = KNIGHT;
            break;
        case 'b':
            type = BISHOP;
            break;
        case 'q':
            type = QUEEN;
            break;
        case 'k':
            type = KING;
            break;
        case 'p':
            type = PAWN;
            break;
    }
    return {type, square_number, color};
}

char piece_to_char(Piece piece) {
    char piece_char = '.';
    switch(piece.type) {
        case EMPTY:
            return piece_char;
        case ROOK:
            piece_char = 'r';
            break;
        case KNIGHT:
            piece_char = 'n';
            break;
        case BISHOP:
            piece_char = 'b';
            break;
        case QUEEN:
            piece_char = 'q';
            break;
        case KING:
            piece_char = 'k';
            break;
        case PAWN:
            piece_char = 'p';
            break;
    }
    if(piece.color == WHITE) {
        piece_char = toupper(piece_char);
    }
    return piece_char;
}

PieceType char_to_piece_type(char piece_char) {
    switch (tolower(piece_char)) {
        case 'r':
            return ROOK;
        case 'n':
            return KNIGHT;
        case 'b':
            return BISHOP;
        case 'q':
            return QUEEN;
        case 'k':
            return KING;
        case 'p':
            return PAWN;
        default:
            return EMPTY;
    }
}

int algebraic_to_square(const string& alg) {
    if (alg == "-") return -1;
    return (alg[0] - 'a') + 8 * (alg[1] - '1');
}

string square_to_algebraic(int square) {
    if (square < 0 || square > 63) return "-";  // Invalid
    char file = 'a' + (square % 8);
    char rank = '1' + (square / 8);
    return string() + file + rank;
}

GameState parse_full_fen(string fen) {
    GameState state;
    state.pieces.clear();

    istringstream iss(fen);
    string boardPart, activeColor, castling, enpassant, halfmove, fullmove;

    iss >> boardPart >> activeColor >> castling >> enpassant >> halfmove >> fullmove;

    // Parse piece positions
    int square = 56;
    for (char c : boardPart) {
        if (c == '/') {
            square -= 16;
        } else if (isdigit(c)) {
            square += c - '0';
        } else {
            Color color = isupper(c) ? WHITE : BLACK;
            PieceType type;
            switch (tolower(c)) {
                case 'p': 
                    type = PAWN; 
                    break;
                case 'n': 
                    type = KNIGHT; 
                    break;
                case 'b': 
                    type = BISHOP; 
                    break;
                case 'r': 
                    type = ROOK; 
                    break;
                case 'q': 
                    type = QUEEN; 
                    break;
                case 'k': 
                    type = KING; 
                    break;
                default: type = EMPTY;
            }
            state.pieces.push_back({ type, square, color });
            square++;
        }
    }

    // Parse metadata
    state.side_to_move = (activeColor == "w") ? WHITE : BLACK;

    state.castle_rights = 0;
    if(castling.find('K') != string::npos) state.castle_rights = state.castle_rights | WHITE_KINGSIDE;
    if(castling.find('Q') != string::npos) state.castle_rights = state.castle_rights | WHITE_QUEENSIDE;
    if(castling.find('k') != string::npos) state.castle_rights = state.castle_rights | BLACK_KINGSIDE;
    if(castling.find('q') != string::npos) state.castle_rights = state.castle_rights | BLACK_QUEENSIDE;

    state.en_passant_square = algebraic_to_square(enpassant);
    state.halfmove_clock = stoi(halfmove);
    state.fullmove_number = stoi(fullmove);

    state.occupancy = get_occupancy(state.pieces);
    return state;
}

void print_board_array(Piece board[64]) {
    cout << "\n";
    for(int i = 0; i > 64; i++) {
        if (i % 8 == 0) {
            cout << "\n";
        }
        cout << piece_to_char(board[i]);
    }
    cout << endl;
}

void print_bitboard(U64 board) {
    cout << "\n";
    for (int row = 7; row >= 0; --row) {
        for (int col = 0; col < 8; ++col) {
            int index = row * 8 + col;
            cout << ((board >> index) & 1ULL);
            cout << " ";
        }
        cout << "\n";
    }
    cout << endl;
} 

string piece_type_to_string(PieceType piece_type) {
    switch(piece_type) {
        case PAWN: 
            return "Pawn";
        case ROOK:
            return "Rook";
        case KNIGHT:
            return "Knight";
        case BISHOP:
            return "Bishop";
        case KING:
            return "King";
        case QUEEN:
            return "Queen";
    }
    return "EMPTY";
}

string move_type_to_string(MoveType move_type) {
    switch(move_type) {
        case PUSH: 
            return "Push";
        case DOUBLE_PUSH:
            return "Double Push";
        case ENPASSANT:
            return "En Passant";
        case CAPTURE:
            return "Capture";
        case QUIET:
            return "Quiet";
        case CASTLE:
            return "Castle";
        case PROMOTION:
            return "Promotion";
        case CAPTURE_AND_PROMOTION:
            return "Capture and Promotion";
    }
    return "None";
}

std::ostream& operator<<(std::ostream& os, const Piece& piece) {
    os << (piece.color == 1 ? "White " : "Black ") << piece_to_char(piece) << " on square " << square_to_algebraic(piece.square_number) << '\n';
    return os;
}

std::ostream& operator<<(std::ostream& os, const Move& move) {
    string promotion_message = " to ";
    promotion_message += piece_type_to_string(move.promotion);
    promotion_message += " | ";
    os << '\n' << piece_type_to_string(move.piece) << " from " << square_to_algebraic(move.from) << " to " << square_to_algebraic(move.to) << '\n';
    os << "Move type: " << move_type_to_string(move.type) << (move.type == PROMOTION? promotion_message : " | ") << "Capturing " << piece_type_to_string(move.captured) << '\n';
    return os;
}

std::ostream& operator<<(std::ostream& os, const GameState& state) {
    char board[64] = {};
    for (Piece piece : state.pieces) {
        board[piece.square_number] = piece_to_char(piece);
    }
    os << '\n';
    for (int row = 7; row >= 0; --row) {
        os << row + 1 << "   ";
        for (int col = 0; col < 8; ++col) {
            int i = row * 8 + col;
            os << (board[i] == 0 ? '.' : board[i]) << ' ';
        }
        os << '\n';
    }

    os << "\n    a b c d e f g h\n";

    os << (state.side_to_move == 1 ? "White" : "Black") << " to move." << '\n';
    os << "Black can castle?\nKingside: " << ((state.castle_rights & WHITE_KINGSIDE)? "Yes" : "No") << " | Queenside: " << ((state.castle_rights & WHITE_QUEENSIDE)? "Yes" : "No") << '\n';
    os << "White can castle?\nKingside: " << ((state.castle_rights & BLACK_KINGSIDE)? "Yes" : "No") << " | Queenside: " << ((state.castle_rights & BLACK_QUEENSIDE)? "Yes" : "No") << '\n';
    os << "En passant square: " << square_to_algebraic(state.en_passant_square) << " | Fullmove clock: " << state.fullmove_number << " | Halfmove clock: " << state.halfmove_clock << '\n';
    return os;
}

std::ostream& operator<<(std::ostream& os, const MoveCounter& counter) {
    os << "\n";
    os << counter.move_count << " Moves\n";
    os << counter.capture_count << " Captures\n";
    os << counter.en_passant_count << " En Passants\n";
    os << counter.castle_count << " Castles\n";
    os << counter.promotion_count << " Promotions\n";
    os << counter.check_count << " Checks\n";
    os << counter.checkmate_count << " Checkmates\n";
    return os;
}

Occupancy get_occupancy(vector<Piece>& pieces) {
    Occupancy occupancy;
    for(Piece piece : pieces) {
        occupancy.all = occupancy.all | (1ULL << piece.square_number);
        if(piece.color == WHITE) occupancy.colors[W_OCC] = occupancy.colors[W_OCC] | (1ULL << piece.square_number);
        if(piece.color == BLACK) occupancy.colors[B_OCC] = occupancy.colors[B_OCC] | (1ULL << piece.square_number);
    }
    occupancy.colors[E_OCC] = ~occupancy.all;
    return occupancy;
}

vector<int> get_set_bit_positions(U64 bb) {
    vector<int> positions;
    while(bb) {
        int i = count_zeros(bb);
        positions.push_back(i);
    }
    return positions;
}

PieceType get_piece_type_at_square(vector<Piece>& pieces, int square_number) {
    for (Piece& piece : pieces) {
        if(square_number == piece.square_number) {
            return piece.type;
        }
    }
    return EMPTY;
}

vector<Move> get_pawn_attacks(GameState& state, Piece& piece) {
    vector<Move> attacks;
    int attack_right = piece.square_number + 9 * piece.color;
    int attack_left = piece.square_number + 7 * piece.color;
    U64 opposing_occupancy = state.occupancy.colors[(-piece.color) + 1];
    bool pawn_can_attack_right = (
        // Not out of bounds
        ((piece.color == WHITE && ((piece.square_number + 1) % 8) != 0) || (piece.color == BLACK && ((piece.square_number) % 8) != 0))
        // And there is an opposing piece to capture
        && (opposing_occupancy & (1ULL << attack_right) || attack_right == state.en_passant_square)
    );

    bool pawn_can_attack_left = (
        ((piece.color == BLACK && ((piece.square_number + 1) % 8) != 0) || (piece.color == WHITE && ((piece.square_number) % 8) != 0))
        && (opposing_occupancy & (1ULL << attack_left) || attack_left == state.en_passant_square)
    );
    if((piece.square_number > 47 && piece.color == WHITE) || (piece.square_number < 16 && piece.color == BLACK)) {
        if(pawn_can_attack_left) {
            for(PieceType piece_promotion : legal_promotions) {
                PieceType captured_piece = get_piece_type_at_square(state.pieces, attack_left);
                attacks.push_back({piece.square_number, attack_left, piece.type, captured_piece, piece_promotion, CAPTURE_AND_PROMOTION});
            }
        }
        if(pawn_can_attack_right) {
            for(PieceType piece_promotion : legal_promotions) {
                PieceType captured_piece = get_piece_type_at_square(state.pieces, attack_right);
                attacks.push_back({piece.square_number, attack_right, piece.type, captured_piece, piece_promotion, CAPTURE_AND_PROMOTION});
            }
        }
        return attacks;
    }
     if(pawn_can_attack_right) {
        PieceType attacked;
        MoveType move_type;
        if(state.en_passant_square == attack_right) {
            attacked = PAWN;
            move_type = ENPASSANT;
        } else {
            attacked = get_piece_type_at_square(state.pieces, attack_right);
            move_type = CAPTURE;
        }
        attacks.push_back({piece.square_number, attack_right, piece.type, attacked, EMPTY, move_type});
    }
    if(pawn_can_attack_left) {
        PieceType attacked;
        MoveType move_type;
        if(state.en_passant_square == attack_left) {
            attacked = PAWN;
            move_type = ENPASSANT;
        } else {
            attacked = get_piece_type_at_square(state.pieces, attack_left);
            move_type = CAPTURE;
        }
        attacks.push_back({piece.square_number, attack_left, piece.type, attacked, EMPTY, move_type});
    }
    return attacks;
}

vector<Move> get_pawn_moves(GameState& state, Piece& piece) {
    vector<Move> pawn_moves;
    int one_ahead_pawn = piece.square_number + 8 * piece.color;
    int attack_right = piece.square_number + 9 * piece.color;
    int attack_left = piece.square_number + 7 * piece.color;
    bool pawn_can_push = state.occupancy.colors[E_OCC] & (1ULL << one_ahead_pawn);

    U64 opposing_occupancy = state.occupancy.colors[(-piece.color) + 1];
    bool pawn_can_attack_right = (
        // Not out of bounds
        ((piece.color == WHITE && ((piece.square_number + 1) % 8) != 0) || (piece.color == BLACK && ((piece.square_number) % 8) != 0))
        // And there is an opposing piece to capture
        && (opposing_occupancy & (1ULL << attack_right) || attack_right == state.en_passant_square)
    );

    bool pawn_can_attack_left = (
        ((piece.color == BLACK && ((piece.square_number + 1) % 8) != 0) || (piece.color == WHITE && ((piece.square_number) % 8) != 0))
        && (opposing_occupancy & (1ULL << attack_left) || attack_left == state.en_passant_square)
        );

    // Promotions
    if((piece.square_number > 47 && piece.color == WHITE) || (piece.square_number < 16 && piece.color == BLACK)) {
        if(pawn_can_push) {
            int promotion = one_ahead_pawn;
            for(PieceType piece_promotion : legal_promotions) {
                pawn_moves.push_back({piece.square_number, promotion, piece.type, EMPTY, piece_promotion, PROMOTION});
            }
        }
        if(pawn_can_attack_left) {
            for(PieceType piece_promotion : legal_promotions) {
                PieceType captured_piece = get_piece_type_at_square(state.pieces, attack_left);
                pawn_moves.push_back({piece.square_number, attack_left, piece.type, captured_piece, piece_promotion, CAPTURE_AND_PROMOTION});
            }
        }
        if(pawn_can_attack_right) {
            for(PieceType piece_promotion : legal_promotions) {
                PieceType captured_piece = get_piece_type_at_square(state.pieces, attack_right);
                pawn_moves.push_back({piece.square_number, attack_right, piece.type, captured_piece, piece_promotion, CAPTURE_AND_PROMOTION});
            }
        }
        return pawn_moves;
    }

    // Pushes and Double Pushes from starting squares
    if((piece.square_number > 47 && piece.color == BLACK) || (piece.square_number < 16 && piece.color == WHITE)) {
        if(pawn_can_push) {
            //Double push only if single push is legal
            int two_ahead_pawn = piece.square_number + 16 * piece.color;
            bool pawn_can_double_push = state.occupancy.colors[E_OCC] & (1ULL << two_ahead_pawn);
            if(pawn_can_double_push) {
                pawn_moves.push_back({piece.square_number, two_ahead_pawn, piece.type, EMPTY, EMPTY, DOUBLE_PUSH});
            }
        }
    }
    // Captures, including en passant
    if(pawn_can_attack_right) {
        PieceType attacked;
        MoveType move_type;
        if(state.en_passant_square == attack_right) {
            attacked = PAWN;
            move_type = ENPASSANT;
        } else {
            attacked = get_piece_type_at_square(state.pieces, attack_right);
            move_type = CAPTURE;
        }
        pawn_moves.push_back({piece.square_number, attack_right, piece.type, attacked, EMPTY, move_type});
    }
    if(pawn_can_attack_left) {
        PieceType attacked;
        MoveType move_type;
        if(state.en_passant_square == attack_left) {
            attacked = PAWN;
            move_type = ENPASSANT;
        } else {
            attacked = get_piece_type_at_square(state.pieces, attack_left);
            move_type = CAPTURE;
        }
        pawn_moves.push_back({piece.square_number, attack_left, piece.type, attacked, EMPTY, move_type});
    }
    if(pawn_can_push) {
        pawn_moves.push_back({piece.square_number, one_ahead_pawn, piece.type, EMPTY, EMPTY, PUSH});
    }
    return pawn_moves;
}

vector<Move> get_castle_moves(GameState& state, Occupancy& occupancy, Piece& piece) {
    vector<Move> rook_moves;
    // Castling
    if(state.side_to_move == WHITE) {
        bool can_castle_kingside = (state.castle_rights & WHITE_KINGSIDE) && ((state.occupancy.colors[E_OCC] & WK_GAP) == WK_GAP) && piece.square_number == ROOK_WK;
        bool can_castle_queenside = (state.castle_rights & WHITE_QUEENSIDE) && ((state.occupancy.colors[E_OCC] & WQ_GAP) == WQ_GAP) && piece.square_number == ROOK_WQ;
        if(can_castle_kingside) {
            rook_moves.push_back({piece.square_number, CASTLE_WK, piece.type, EMPTY, EMPTY, CASTLE});
        } 
        if(can_castle_queenside) {
            rook_moves.push_back({piece.square_number, CASTLE_WQ, piece.type, EMPTY, EMPTY, CASTLE});
        }
    }
    if(state.side_to_move == BLACK) {
        bool can_castle_kingside = (state.castle_rights & BLACK_KINGSIDE) && ((state.occupancy.colors[E_OCC] & BK_GAP) == BK_GAP) && piece.square_number == ROOK_BK;
        bool can_castle_queenside = (state.castle_rights & BLACK_QUEENSIDE) && ((state.occupancy.colors[E_OCC] & BQ_GAP) == BQ_GAP) && piece.square_number == ROOK_BQ;
        if(can_castle_kingside) {
            rook_moves.push_back({piece.square_number, CASTLE_BK, piece.type, EMPTY, EMPTY, CASTLE});
        } 
        if(can_castle_queenside) {
            rook_moves.push_back({piece.square_number, CASTLE_BQ, piece.type, EMPTY, EMPTY, CASTLE});
        }
    }
    
    return rook_moves;

}

vector<Move> get_all_pseudo_legal_captures(GameState& state) {
    vector<Move> captures;
    for(Piece& piece : state.pieces) {
        if(piece.color != state.side_to_move) continue;

        if(piece.type == PAWN) {
            vector<Move> pawn_moves = get_pawn_attacks(state, piece);
            captures.reserve(pawn_moves.size());
            captures.insert(captures.end(), captures.begin(), captures.end());
        }

        U64 piece_vision_set = get_piece_attack_vision(piece.square_number, state.occupancy.all, piece.type);
        U64 capture_move_set = piece_vision_set & state.occupancy.colors[-piece.color + 1];
        while(capture_move_set) {
            int move_to = count_zeros(capture_move_set);
            PieceType captured = get_piece_type_at_square(state.pieces, move_to);
            captures.push_back({piece.square_number, move_to, piece.type, captured, EMPTY, CAPTURE});
            capture_move_set = capture_move_set & (capture_move_set - 1);
        }
    }
    return captures;
}

vector<Move> get_all_pseudo_legal_moves(GameState& state) {
    // Create occupancy bb
    vector<Move> legal_moves;
    for(Piece& piece : state.pieces) {
        if(piece.color != state.side_to_move) continue;

        if(piece.type == PAWN) {
            vector<Move> pawn_moves = get_pawn_moves(state, piece);
            legal_moves.reserve(pawn_moves.size());
            legal_moves.insert(legal_moves.end(), pawn_moves.begin(), pawn_moves.end());
        }

        if(piece.type == ROOK && state.castle_rights) {
            vector<Move> castle_moves = get_castle_moves(state, state.occupancy, piece);
            legal_moves.reserve(castle_moves.size());
            legal_moves.insert(legal_moves.end(), castle_moves.begin(), castle_moves.end());
        }

        U64 piece_vision_set = get_piece_attack_vision(piece.square_number, state.occupancy.all, piece.type);
        
        // Quiet moves
        U64 quiet_move_set = piece_vision_set & state.occupancy.colors[E_OCC];

        // Captures
        U64 capture_move_set = piece_vision_set & state.occupancy.colors[-piece.color + 1];

        while(quiet_move_set) {
            int move_to = count_zeros(quiet_move_set);
            legal_moves.push_back({piece.square_number, move_to, piece.type, EMPTY, EMPTY, QUIET});
            quiet_move_set = quiet_move_set & (quiet_move_set - 1);
        }
        while(capture_move_set) {
            int move_to = count_zeros(capture_move_set);
            PieceType captured = get_piece_type_at_square(state.pieces, move_to);
            legal_moves.push_back({piece.square_number, move_to, piece.type, captured, EMPTY, CAPTURE});
            capture_move_set = capture_move_set & (capture_move_set - 1);
        }
    }
    return legal_moves;
}

void move_piece(GameState& state, int pi, int from, int to) {
    if(pi == -1) {
        cout << '\n'<< state;
    }
    state.pieces[pi].square_number = to;
    U64& color_occupancy = state.occupancy.colors[state.pieces[pi].color + 1];
    color_occupancy = color_occupancy & ~(1ULL << from);
    color_occupancy = color_occupancy | (1ULL << to);
    state.occupancy.all = state.occupancy.colors[W_OCC] | state.occupancy.colors[B_OCC];
    state.occupancy.colors[E_OCC] = ~state.occupancy.all;
}

void handle_unmove(GameState& state, int pi, int from, int to, UnMakeInfo info) {
    move_piece(state, pi, from, to);
    state.side_to_move = Color(-state.side_to_move);
    state.en_passant_square = info.en_passant_square;
    state.castle_rights = info.castling_rights;
    state.halfmove_clock = info.halfmove_clock;
}

void handle_move(GameState& state, int pi, int from, int to) {
    move_piece(state, pi, from, to);
    state.side_to_move = Color(-state.side_to_move);
    state.en_passant_square = -1;
    if(state.pieces[pi].type == ROOK) {
        // Short, white
        if(from == ROOK_WK) {
            state.castle_rights = state.castle_rights & ~(WHITE_KINGSIDE);
        }
        // Long, white
        else if(from == ROOK_WQ) {
            state.castle_rights = state.castle_rights & ~(WHITE_QUEENSIDE);
        }
        // Short, black
        else if(from == ROOK_BK) {
            state.castle_rights = state.castle_rights & ~(BLACK_KINGSIDE);
        }
        // Long, black
        else if(from == ROOK_BQ) {
            state.castle_rights = state.castle_rights & ~(BLACK_QUEENSIDE);
        }
    }
    if(state.pieces[pi].type == KING) {
        if(state.pieces[pi].color == WHITE) {
            state.castle_rights = state.castle_rights & ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
        } else {
            state.castle_rights = state.castle_rights & ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
        }
    }
}

void set_occupancy(GameState& state, int sq, Color color) {
    U64 color_occupancy = state.occupancy.colors[color + 1];
    color_occupancy = color_occupancy | (1ULL << sq);
    state.occupancy.colors[color + 1] = color_occupancy;
    state.occupancy.all = state.occupancy.colors[W_OCC] | state.occupancy.colors[B_OCC];
    state.occupancy.colors[E_OCC] = ~state.occupancy.all;
}

void remove_occupancy(GameState& state, int sq, Color color) {
    U64 color_occupancy = state.occupancy.colors[color + 1];
    color_occupancy = color_occupancy & ~(1ULL << sq);
    state.occupancy.colors[color + 1] = color_occupancy;
    state.occupancy.all = state.occupancy.colors[W_OCC] | state.occupancy.colors[B_OCC];
    state.occupancy.colors[E_OCC] = ~state.occupancy.all;
}

void unmake_move(GameState& state, const Move& move, UnMakeInfo info) {
    int mpi = -1;
    int cpi = -1;
    int ki = -1;
    int piece_index = 0;
    Color just_moved = Color(-state.side_to_move); // Opposite color for replacing captured pieces NOT SIDE TO MOVE
    Color opp_of_just_moved = state.side_to_move;

    for(Piece& piece : state.pieces) {
        if(piece.square_number == move.to) {
            mpi = piece_index;
        } 
        if(piece.type == KING && piece.color == just_moved) {
            ki = piece_index;
        }
        if(mpi > -1 && (ki > -1 || !(move.type == CASTLE))){
            break;
        } 
        piece_index ++;
    }

    switch(move.type) {
        case CAPTURE_AND_PROMOTION: {
            handle_unmove(state, mpi, move.to, move.from, info);
            state.pieces[mpi].type = PAWN;
            state.pieces.push_back({move.captured, move.to, opp_of_just_moved});
            set_occupancy(state, move.to, opp_of_just_moved);
            return;
        }
        case PROMOTION: {
            handle_unmove(state, mpi, move.to, move.from, info);
            state.pieces[mpi].type = PAWN;
            return;
        }
        case ENPASSANT: {
            handle_unmove(state, mpi, move.to, move.from, info);
            state.pieces.push_back({PAWN, (move.to + 8 * opp_of_just_moved), opp_of_just_moved});
            // Add back occupancy for captured pawn
            set_occupancy(state, (move.to + 8 * opp_of_just_moved), opp_of_just_moved);
            return;
        }
        case CAPTURE: {
            handle_unmove(state, mpi, move.to, move.from, info);
            state.pieces.push_back({move.captured, move.to, opp_of_just_moved});
            set_occupancy(state, move.to, opp_of_just_moved);
            return;
        }
        case CASTLE: {
            handle_unmove(state, mpi, move.to, move.from, info);
            remove_occupancy(state, state.pieces[ki].square_number, state.pieces[ki].color);
            if(state.pieces[ki].color == WHITE) {
                state.pieces[ki].square_number = WHITE_KING_SQ;
                set_occupancy(state, WHITE_KING_SQ, WHITE);
            } else {
                state.pieces[ki].square_number = BLACK_KING_SQ;
                set_occupancy(state, BLACK_KING_SQ, BLACK);
            }
            return;
        }
        case DOUBLE_PUSH: {
            handle_unmove(state, mpi, move.to, move.from, info);
            return;
        }
        // Push, Quiet
        default: {
            handle_unmove(state, mpi, move.to, move.from, info);
            return;
        }
    }
}

void make_move(GameState& state, const Move& move) {
    int mpi = -1;
    int cpi = -1;
    int ki = -1;
    int piece_index = 0;

    for(Piece& piece : state.pieces) {
        if(piece.square_number == move.from) {
            mpi = piece_index;
        } 
        if(piece.square_number == move.to || (move.type == ENPASSANT && piece.square_number == (move.to + 8 * piece.color))) {
            cpi = piece_index;
        }
        if(piece.type == KING && piece.color == state.side_to_move) {
            ki = piece_index;
        }
        if(mpi > -1 && (cpi > -1 || move.captured == EMPTY) && (ki > -1 || !(move.type == CASTLE))){
            break;
        } 
        piece_index ++;
    }
    if(move.type == CAPTURE && move.captured == KING) {
        cout << move;
        cout << state;
    }
    switch (move.type) {
        case CAPTURE_AND_PROMOTION: {
            handle_move(state, mpi, move.from, move.to);
            state.pieces[mpi].type = move.promotion;
            remove_occupancy(state, state.pieces[cpi].square_number, state.pieces[cpi].color);
            state.pieces.erase(state.pieces.begin() + cpi);
            return;
        }
        case PROMOTION: {
            handle_move(state, mpi, move.from, move.to);
            state.pieces[mpi].type = move.promotion;
            return;
        }
        case ENPASSANT: {
            handle_move(state, mpi, move.from, move.to);
            // Remove occupancy for captured pawn
            remove_occupancy(state, state.pieces[cpi].square_number, state.pieces[cpi].color);
            state.pieces.erase(state.pieces.begin() + cpi);
            return;
        }
        case CAPTURE: {
            handle_move(state, mpi, move.from, move.to);
            remove_occupancy(state, state.pieces[cpi].square_number, state.pieces[cpi].color);
            state.pieces.erase(state.pieces.begin() + cpi);
            return;
        }
        case CASTLE: {
            if(state.side_to_move == WHITE) {
                state.castle_rights = state.castle_rights & ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
            } else {
                state.castle_rights = state.castle_rights & ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
            }
            handle_move(state, mpi, move.from, move.to);
            int king_start = state.pieces[ki].square_number;
            int king_end;
            // Short, white
            if(move.to == 5) {
                king_end = WK_SHORT_SQ;
            }
            // Long, white
            else if(move.to == 3) {
                king_end = WK_LONG_SQ;
            }
            // Short, black
            else if(move.to == 61) {
                king_end = BK_SHORT_SQ;
            }
            // Long, black
            else if(move.to == 59) {
                king_end = BK_LONG_SQ;
            }
            move_piece(state, ki, king_start, king_end);
            return;
        }
        case DOUBLE_PUSH: {
            handle_move(state, mpi, move.from, move.to);
            state.en_passant_square = move.to - 8 * state.pieces[mpi].color;
            return;
        }
        
    }

    // All quiet moves and single pushes just set the new location
    handle_move(state, mpi, move.from, move.to);
}

U64 get_enemy_attack_vision(GameState& state, Color color) {
    U64 enemy_attack_vision = 0;
    for(Piece& piece : state.pieces) {
        if(piece.color != color) {
            enemy_attack_vision = enemy_attack_vision | get_piece_attack_vision(piece.square_number, state.occupancy.all, piece.type);
        }
    }
    return enemy_attack_vision;
}

bool is_king_in_check(GameState& state, Color color, int king_square, U64 enemy_attack_vision) {
    if(enemy_attack_vision == 0xFFFFFFFFFFFFFFFFULL) {
        enemy_attack_vision = get_enemy_attack_vision(state, color);
        for(Piece& piece : state.pieces) {
            if(piece.type == KING && piece.color == color) {
                king_square = piece.square_number;
            }
            if(piece.type == PAWN && piece.color == -color) {
                enemy_attack_vision = enemy_attack_vision | get_pawn_attack_vision(piece.square_number, piece.color);
            }
        }
    }
    return ((1ULL << king_square) & enemy_attack_vision);
}

bool is_illegal_move(GameState& state, const Move& last_move) {
    Color color = Color(-state.side_to_move);
    U64 enemy_attack_vision = get_enemy_attack_vision(state, color);
    int king_square = -1;
    for(Piece& piece : state.pieces) {
        if(piece.type == KING && piece.color == color) {
            king_square = piece.square_number;
        }
        if(piece.type == PAWN && piece.color == -color) {
            enemy_attack_vision = enemy_attack_vision | get_pawn_attack_vision(piece.square_number, piece.color);
        }
    }

    bool illegal_move = is_king_in_check(state, color, king_square, enemy_attack_vision);

    if(last_move.type == CASTLE && !illegal_move) {
        // Check for castling out of check
        if(last_move.to == CASTLE_WK || last_move.to == CASTLE_WQ) {
            illegal_move = is_king_in_check(state, color, WHITE_KING_SQ, enemy_attack_vision);
        } else {
            illegal_move = is_king_in_check(state, color, BLACK_KING_SQ, enemy_attack_vision);
        }
        if(illegal_move) {
            return illegal_move;
        }
        // Check for castling through check
        if(last_move.to == CASTLE_BK || last_move.to == CASTLE_WK) {
            illegal_move = is_king_in_check(state, color, king_square - 1, enemy_attack_vision);
        } else {
            illegal_move = is_king_in_check(state, color, king_square + 1, enemy_attack_vision);
        }
        // Castling into check should be pruned automatically
        if(illegal_move) {
            //cout << '\n' << state;
            return illegal_move;
        }
    }
    return illegal_move;
}

int prune_illegal_moves(GameState& state, vector<Move>& legal_moves) {
    UnMakeInfo info;
    info.castling_rights = state.castle_rights;
    info.en_passant_square = state.en_passant_square;
    info.halfmove_clock = state.halfmove_clock;
    // cout << '\n' << "Pruning\n" << state;
    int i = 0;
    bool is_illegal;
    for (auto it = legal_moves.begin(); it != legal_moves.end();) {
        const Move& move = *it;
        make_move(state, move);
        is_illegal = is_illegal_move(state, move);
        unmake_move(state, move, info);
        if(is_illegal) {
            it = legal_moves.erase(it);
        } else {
            ++it;
        }
        i++;
    }
    return legal_moves.size();
}

MoveCounter operator+(const MoveCounter& lhs, const MoveCounter& rhs) {
    return {
        lhs.move_count + rhs.move_count,
        lhs.capture_count + rhs.capture_count,
        lhs.en_passant_count + rhs.en_passant_count,
        lhs.castle_count + rhs.castle_count,
        lhs.promotion_count + rhs.promotion_count,
        lhs.check_count + rhs.check_count,
        lhs.checkmate_count + rhs.checkmate_count
    };
}

int operator==(const MoveCounter& lhs, const MoveCounter& rhs) {
    return 
        lhs.move_count == rhs.move_count &&
        lhs.capture_count == rhs.capture_count &&
        lhs.en_passant_count == rhs.en_passant_count &&
        lhs.castle_count == rhs.castle_count &&
        lhs.promotion_count == rhs.promotion_count &&
        lhs.check_count == rhs.check_count &&
        lhs.checkmate_count == rhs.checkmate_count;
}


float quiesce(GameState& state, float alpha, float beta) {
    float static_eval = evaluate(state);

    float best_val = static_eval;
    if(best_val >= beta) {
        return best_val;
    }
    if(best_val > alpha) {
        alpha = best_val;
    }

    vector<Move> all_captures = get_all_pseudo_legal_captures(state);
    prune_illegal_moves(state, all_captures);
    UnMakeInfo info = {state.castle_rights, state.en_passant_square, state.halfmove_clock};

    for(Move& capture : all_captures) {
        make_move(state, capture);
        float score = -quiesce(state, alpha, beta);
        unmake_move(state, capture, info);
        if(score >= beta) {
            return score;
        }
        if(score > best_val) {
            best_val = score;
        }
        if(score > alpha) {
            alpha = score;
        }
        return best_val;
    }
    return best_val;
}

float alpha_beta(GameState& state, float alpha, float beta, int depth, bool& stop) {
    
    if(depth == 0 || stop) {
        return quiesce(state, alpha, beta);
    }
    float best_val = -FLT_MAX;
    vector<Move> all_moves = get_all_legal_moves(state);
    UnMakeInfo info = {state.castle_rights, state.en_passant_square, state.halfmove_clock};
    for(Move move : all_moves) {
        make_move(state, move);
        float score = -alpha_beta(state, -beta, -alpha, depth - 1, stop);
        unmake_move(state, move, info);
        if(score > best_val) {
            best_val = score;
            if(score > alpha) {
                alpha = score;
            }
            if(score >= beta) {
                return best_val;
            }
        }
    }
    return best_val;
}

void stop_after_n_ms(bool& stop, int time_ms) {
    std::this_thread::sleep_for(milliseconds(time_ms));
    stop = true;
}

MoveEval ab_iterative_deepener(GameState& state, bool& stop, int max_time_ms) {
    time_point<system_clock> search_start_time = high_resolution_clock::now();
    MoveEval selected;
    float max_score = -FLT_MAX;
    int depth = 1;

    std::thread SearchStopper(stop_after_n_ms, std::ref(stop), max_time_ms);

    while(!stop) {
        selected = alpha_beta_search(state, depth, stop);
        depth += 1;
    }
    SearchStopper.join();

    return selected;
}

MoveEval alpha_beta_search(GameState& state, int depth, bool& stop) {
    UnMakeInfo info = {state.castle_rights, state.en_passant_square, state.halfmove_clock};
    vector<Move> legal_moves = get_all_legal_moves(state);

    float max_score = -FLT_MAX;
    MoveEval selected;
    cout << "AB Search Depth:" << depth << endl;
    for(const Move& move : legal_moves) {
        make_move(state, move);
        float score = -alpha_beta(state, -FLT_MAX, FLT_MAX, depth, stop);
        unmake_move(state, move, info);
        if(score >= max_score) {
            max_score = score;
            selected = {move, max_score};
            if(max_score == FLT_MAX) {
                return selected;
            }
        }
    }
    return selected;
}

MoveEval negamax_search(GameState& state, int depth) {

    UnMakeInfo info = {state.castle_rights, state.en_passant_square, state.halfmove_clock};
    vector<Move> legal_moves = get_all_pseudo_legal_moves(state);
    U64 legal_move_count = prune_illegal_moves(state, legal_moves);
    
    float max_score = -FLT_MAX;
    MoveEval selected;
    for(const Move& move : legal_moves) {
        make_move(state, move);
        float score = -negamax(state, depth);
        unmake_move(state, move, info);
        if(score > max_score) {
            max_score = score;
            selected = {move, max_score};
        }
    }
    return selected;
}

vector<Move> get_all_legal_moves(GameState& state) {
    vector<Move> legal_moves = get_all_pseudo_legal_moves(state);
    prune_illegal_moves(state, legal_moves);
    return legal_moves;
}

float negamax(GameState& state, int depth) {
    UnMakeInfo info = {state.castle_rights, state.en_passant_square, state.halfmove_clock};
    if(depth == 0) {
        return evaluate(state);
    }
    float max = -FLT_MAX;
    vector<Move> moves = get_all_legal_moves(state);
    for (Move move : moves) {
        make_move(state, move);
        float score = -negamax(state, depth - 1);
        unmake_move(state, move, info);
        if(score > max) {
            max = score;
        }
    }
    return max;
}

float evaluate(GameState& state) {
    // Positive - Side to move winning
    // Negative - Side to move losing
    vector<Move> legal_moves = get_all_pseudo_legal_moves(state);
    vector<Move> pseudo_legal_moves = legal_moves;
    U64 legal_move_count = prune_illegal_moves(state, legal_moves);

    float evaluation = 0;
    if(is_king_in_check(state, state.side_to_move) && legal_move_count == 0) {
        // AVOID BEING IN CHECKMATE - ULTIMATE NEGATIVE VALUE
        return -FLT_MAX;
    } else if(legal_move_count == 0) {
        // Draw
        return 0;
    }

    for(Piece piece : state.pieces) {
        // Count material
        evaluation += calculate_piece_value(piece);
    }
    float low_material_bonus = (MATERIAL_COUNT / state.pieces.size());
    evaluation *= low_material_bonus;
    return evaluation * state.side_to_move;
}

float calculate_piece_value(Piece& piece) {
    float value = PIECE_VALUES[piece.type];
/*
    if(piece.type == KNIGHT || piece.type == PAWN) {
        value += CENTRALITY[piece.square_number] * 0.1;
    }
    if(piece.type == BISHOP) {
        value += DIAGONALITY[piece.square_number] * 0.1;
    }
    if(piece.type == QUEEN) {
        value += CENTRALITY[piece.square_number] * 0.05 + DIAGONALITY[piece.square_number] * 0.05;
    }
    if(piece.type == KING) {
        value -= CENTRALITY[piece.square_number] * 0.1;
        value += KING_SAFETY[piece.square_number] * 0.1;
    }
*/ 
    return value * piece.color;
}

MoveCounter build_perft_move_tree(GameState& state, int depth, GameNode& node) {
    U64 legal_move_count;
    vector<Move> legal_moves;
    MoveCounter move_counter;
    //GameState saved_state = state;
    UnMakeInfo info = {state.castle_rights, state.en_passant_square, state.halfmove_clock};
    MoveCounter check;
    bool king_is_in_check;

    if (depth == 0) {
        king_is_in_check = is_king_in_check(state, Color(state.side_to_move));
        if(king_is_in_check) {
            legal_moves = get_all_pseudo_legal_moves(state);
            legal_move_count = prune_illegal_moves(state, legal_moves);
            if(legal_move_count) {
                check = {0, 0, 0, 0, 0, 1};
            } else {
                check = {0, 0, 0, 0, 0, 1, 1};
            }
        }
        switch (node.move.type) {
            case CAPTURE:
                return MoveCounter({1, 1}) + check;
            case ENPASSANT:
                return MoveCounter({1, 1, 1}) + check;
            case CASTLE:
                return MoveCounter({1, 0, 0, 1}) + check;
            case PROMOTION:
                return MoveCounter({1, 0, 0, 0, 1}) + check;
            case CAPTURE_AND_PROMOTION:
                return MoveCounter({1, 1, 0, 0, 1}) + check;
            default:
                return MoveCounter({1}) + check;
        }
    }

    legal_moves = get_all_pseudo_legal_moves(state);
    legal_move_count = prune_illegal_moves(state, legal_moves);

    for(const Move& move : legal_moves) {
        make_move(state, move);
        node.children.emplace_back(GameNode{state, move});
        GameNode& child = node.children.back();
        MoveCounter child_move_count = build_perft_move_tree(state, depth - 1, child);
        child.move_counter = child_move_count;
        move_counter = move_counter + child_move_count;
        unmake_move(state, move, info);
    }
    return move_counter;
}

bool is_valid_token(string token) {
    return (
        token == "uci" ||
        token == "debug" ||
        token == "isready" ||
        token == "setoption" ||
        token == "register" ||
        token == "ucinewgame" ||
        token == "position" ||
        token == "go" ||
        token == "stop" ||
        token == "ponderhit" ||
        token == "quit"
    );
}

Move verbose_to_move(const string& verbose, vector<Piece>& pieces) {
    string from_algabraic = verbose.substr(0, 2);
    string to_algabraic = verbose.substr(2, 2);
    char promotion = '\0';

    if(verbose.size() > 4) {
        promotion = verbose[4];
    }

    int from_square = algebraic_to_square(from_algabraic);
    int to_square = algebraic_to_square(to_algabraic);
    PieceType moving = get_piece_type_at_square(pieces, from_square);
    PieceType capturing = get_piece_type_at_square(pieces, to_square);
    MoveType move_type = QUIET;
    Move move;

    if(moving == KING) {
        if(from_square == WHITE_KING_SQ) {
            if(to_square == WK_SHORT_SQ) {
                return {ROOK_WK, CASTLE_WK, ROOK, EMPTY, EMPTY, CASTLE};
            } 
            if(to_square == WK_LONG_SQ) {
                return {ROOK_WQ, CASTLE_WQ, ROOK, EMPTY, EMPTY, CASTLE};
            }
        }
        if (from_square == BLACK_KING_SQ) {
            if (to_square == BK_SHORT_SQ) {
                return {ROOK_BK, CASTLE_BK, ROOK, EMPTY, EMPTY, CASTLE};
            } 
            if (to_square == BK_LONG_SQ) {
                return {ROOK_BQ, CASTLE_BQ, ROOK, EMPTY, EMPTY, CASTLE};
            }
        }
    }

    // Check for PUSH, DOUBLE_PUSH, ENPASSANT, CAPTURE, QUIET, CASTLE, PROMOTION, CAPTURE_AND_PROMOTION
    // Pawn capture, check for ep
    if(moving == PAWN && !promotion) {
        if(((from_square - to_square) % 8) != 0) {
            if(capturing == EMPTY) {
                return {from_square, to_square, PAWN, PAWN, EMPTY, ENPASSANT};
            }
        } else {
            if(abs(from_square - to_square) == 8) {
                return {from_square, to_square, PAWN, EMPTY, EMPTY, PUSH};
            }
            if(abs(from_square - to_square) == 16) {
                return {from_square, to_square, PAWN, EMPTY, EMPTY, DOUBLE_PUSH};
            }
        }
    }
    if(capturing != EMPTY && !promotion) {
        return {from_square, to_square, moving, capturing, EMPTY, CAPTURE};
    } else if (capturing != EMPTY && promotion) {
        return {from_square, to_square, moving, capturing, char_to_piece_type(promotion), CAPTURE_AND_PROMOTION};
    } else if (promotion) {
        return {from_square, to_square, moving, EMPTY, char_to_piece_type(promotion), PROMOTION};
    }

    return {from_square, to_square, moving, EMPTY, EMPTY, QUIET};
}

string move_to_verbose(Move& move) {
    string verbose = "";
    if(move.type == CASTLE) {
        if(move.from == ROOK_WK || move.from == ROOK_WQ) {
            verbose += square_to_algebraic(WHITE_KING_SQ);
            if(move.to == CASTLE_WK) {
                verbose += square_to_algebraic(WK_SHORT_SQ);
            }
            if(move.to == CASTLE_WQ) {
                verbose += square_to_algebraic(WK_LONG_SQ);
            }
        }
        if(move.from == ROOK_BK || move.from == ROOK_BQ) {
            verbose += square_to_algebraic(BLACK_KING_SQ);
            if(move.to == CASTLE_BK) {
                verbose += square_to_algebraic(BK_SHORT_SQ);
            }
            if(move.to == CASTLE_BQ) {
                verbose += square_to_algebraic(BK_LONG_SQ);
            }
        }
        return verbose;
    }
    verbose += square_to_algebraic(move.from);
    verbose += square_to_algebraic(move.to);
    if(move.type == PROMOTION || move.type == CAPTURE_AND_PROMOTION) {
        verbose += piece_to_char({move.promotion, move.from, BLACK});
    }
    return verbose;
}
