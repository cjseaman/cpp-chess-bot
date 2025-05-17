#include "../include/chess.hpp"

#define U64 uint64_t

U64 piece_vision[7][64];
U64 piece_bnb[7][64];
U64 behind[64][64];

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

void generate_behind() {
    for(int i = 0; i < 64; i++) {
        for(int j = 0; j < 64; j++) {
            behind[i][j] = squares_behind(i, j);
        }
    }
}

void generate_piece_vision_maps() {
    cout << "Generating piece vision..." << endl;
    for(int i = 0; i < 64; i++) {
        cout << i << " ";
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
    cout << "Generation finished." << endl;
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
            vision = vision & ~behind[p][sq];
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

    state.white_can_castle_kingside  = castling.find('K') != string::npos;
    state.white_can_castle_queenside = castling.find('Q') != string::npos;
    state.black_can_castle_kingside  = castling.find('k') != string::npos;
    state.black_can_castle_queenside = castling.find('q') != string::npos;

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
    os << piece_type_to_string(move.piece) << " from " << square_to_algebraic(move.from) << " to " << square_to_algebraic(move.to) << '\n';
    os << "Move type: " << move_type_to_string(move.type) << (move.type == PROMOTION? promotion_message : " | ") << "Capturing " << piece_type_to_string(move.captured) << '\n';
    // TODO finish this.
    return os;
}

std::ostream& operator<<(std::ostream& os, const GameState& state) {
    char board[64] = {};
    for (Piece piece : state.pieces) {
        os << piece;
        board[piece.square_number] = piece_to_char(piece);
    }
    for (int row = 7; row >= 0; --row) {
        cout << row + 1 << "   ";
        for (int col = 0; col < 8; ++col) {
            int i = row * 8 + col;
            os << (board[i] == 0 ? '.' : board[i]) << ' ';
        }
        cout << '\n';
    }

    os << "\n    a b c d e f g h\n";

    os << (state.side_to_move == 1 ? "White" : "Black") << " to move." << '\n';
    os << "Black can castle?\nKingside: " << (state.black_can_castle_kingside? "Yes" : "No") << " | Queenside: " << (state.black_can_castle_queenside? "Yes" : "No") << '\n';
    os << "White can castle?\nKingside: " << (state.white_can_castle_kingside? "Yes" : "No") << " | Queenside: " << (state.white_can_castle_queenside? "Yes" : "No") << '\n';
    os << "En passant square: " << square_to_algebraic(state.en_passant_square) << " | Fullmove clock: " << state.fullmove_number << " | Halfmove clock: " << state.halfmove_clock << '\n';
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

vector<Move> get_pawn_moves(GameState& state, Piece& piece) {
    vector<Move> pawn_moves;
    int one_ahead_pawn = piece.square_number + 8 * piece.color;
    int attack_right = piece.square_number + 9 * piece.color;
    int attack_left = piece.square_number + 7 * piece.color;
    bool pawn_can_push = state.occupancy.colors[E_OCC] & (1ULL << one_ahead_pawn);

    U64 opposing_occupancy = state.occupancy.colors[-piece.color + 1];
    bool pawn_can_attack_right = opposing_occupancy & (1ULL << attack_right) || attack_right == state.en_passant_square;
    bool pawn_can_attack_left = opposing_occupancy & (1ULL << attack_left) || attack_left == state.en_passant_square;

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
        bool can_castle_kingside = state.white_can_castle_kingside && ((state.occupancy.colors[E_OCC] & WK_GAP) == WK_GAP) && piece.square_number == 7;
        bool can_castle_queenside = state.white_can_castle_queenside && ((state.occupancy.colors[E_OCC] & WQ_GAP) == WQ_GAP) && piece.square_number == 0;
        if(can_castle_kingside) {
            rook_moves.push_back({piece.square_number, 5, piece.type, EMPTY, EMPTY, CASTLE});
        } 
        if(can_castle_queenside) {
            rook_moves.push_back({piece.square_number, 3, piece.type, EMPTY, EMPTY, CASTLE});
        }
    }
    if(state.side_to_move == BLACK) {
        bool can_castle_kingside = state.black_can_castle_kingside && ((state.occupancy.colors[E_OCC] & BK_GAP) == BK_GAP) && piece.square_number == 63;
        bool can_castle_queenside = state.black_can_castle_queenside && ((state.occupancy.colors[E_OCC] & BQ_GAP) == BQ_GAP) && piece.square_number == 56;
        if(can_castle_kingside) {
            rook_moves.push_back({piece.square_number, 5, piece.type, EMPTY, EMPTY, CASTLE});
        } 
        if(can_castle_queenside) {
            rook_moves.push_back({piece.square_number, 3, piece.type, EMPTY, EMPTY, CASTLE});
        }
    }
    
    return rook_moves;

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

        if(piece.type == ROOK) {
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
    state.pieces[pi].square_number = to;
    U64& color_occupancy = state.occupancy.colors[state.pieces[pi].color + 1];
    color_occupancy = color_occupancy & ~(1ULL << from);
    color_occupancy = color_occupancy | (1ULL << to);
    state.occupancy.all = state.occupancy.colors[W_OCC] | state.occupancy.colors[B_OCC];
    state.occupancy.colors[E_OCC] = ~state.occupancy.all;
}

void make_move(GameState& state, Move& move) {
    int mpi = -1;
    int cpi = -1;
    int ki = -1;
    int piece_index = 0;

    for(Piece& piece : state.pieces) {
        if(piece.square_number == move.from) {
            mpi = piece_index;
        } 
        if(piece.square_number == move.to || (move.type == ENPASSANT && piece.square_number == (move.to - 8 * piece.color))) {
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

    switch (move.type) {
        case CAPTURE_AND_PROMOTION: {
            state.pieces.erase(state.pieces.begin() + cpi);
            move_piece(state, mpi, move.from, move.to);
            state.pieces[mpi].type = move.promotion;
            return;
        }
        case PROMOTION: {
            move_piece(state, mpi, move.from, move.to);
            state.pieces[mpi].type = move.promotion;
            return;
        }
        case ENPASSANT: {
            state.pieces.erase(state.pieces.begin() + cpi);
            move_piece(state, mpi, move.from, move.to);
            // Remove occupancy for captured pawn
            U64& color_occupancy = state.occupancy.colors[state.pieces[cpi].color + 1];
            color_occupancy = color_occupancy & ~(1ULL << move.to - 8 * state.pieces[cpi].color);
            state.occupancy.all = state.occupancy.colors[W_OCC] | state.occupancy.colors[B_OCC];
            state.occupancy.colors[E_OCC] = ~state.occupancy.all;
            return;
        }
        case CAPTURE: {
            state.pieces.erase(state.pieces.begin() + cpi);
            move_piece(state, mpi, move.from, move.to);
            return;
        }
        case CASTLE: {
            move_piece(state, mpi, move.from, move.to);
            int king_start = state.pieces[ki].square_number;
            int king_end;
            // Short, white
            if(move.to == 5) {
                king_end = 6;
            }
            // Long, white
            else if(move.to == 3) {
                king_end = 2;
            }
            // Short, black
            else if(move.to == 61) {
                king_end = 62;
            }
            // Long, black
            else if(move.to == 59) {
                king_end = 58;
            }
            move_piece(state, ki, king_start, king_end);
            return;
        }
        case DOUBLE_PUSH: {
            state.en_passant_square = move.to - 8 * state.pieces[mpi].color;
            move_piece(state, mpi, move.from, move.to);
            return;
        }
        
    }

    // All quiet moves and single pushes just set the new location
    move_piece(state, mpi, move.from, move.to);
}

void unmake_move(GameState& state, Move& move) {
    //TODO
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

bool is_illegal_move(GameState& state, Move& last_move) {
    Color color = state.side_to_move;
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
    bool illegal_move = ((1ULL << king_square) & enemy_attack_vision);

    if(last_move.type == CASTLE && !illegal_move) {
        // Long white
        if(last_move.from == 0) {
            illegal_move = enemy_attack_vision & (WQ_GAP | (1ULL << 4));
        }
        // Short white
        if(last_move.from == 7) {
            illegal_move = enemy_attack_vision & (WK_GAP | (1ULL << 4));
        }
        // Long black
        if(last_move.from == 56) {
            illegal_move = enemy_attack_vision & (BQ_GAP | (1ULL << 60));
        }
        // Short black
        if(last_move.from == 63) {
            illegal_move = enemy_attack_vision & (BK_GAP | (1ULL << 60));
        }
    }
    return illegal_move;
}

int prune_illegal_moves(GameState& state, vector<Move>& legal_moves) {
    vector<int> illegal;
    GameState saved_state = state;
    int ii;
    for(Move move : legal_moves) {
        make_move(state, move);
        if(is_illegal_move(state, move)) {
            illegal.push_back(ii);
        }
        ii++;
        state = saved_state;
    }
    for(int i : illegal) {
        legal_moves.erase(legal_moves.begin() + i);
    }
    return legal_moves.size();
}
 