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
                result |= 1ULL << sq;
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

// Piece class declarations. Pieces have vision which tells the move process where they may go according to the rules. This accounts for out of bounds but does not account for collision or checks. 

U64 get_pawn_attack_vision(int square_number, Color color, Occupancy occupancy, int en_passant_square) {
    U64 position = 1ULL << square_number;
    U64 vision;
    U64 attack_occupancy;
    if (color == WHITE) {
        vision = east_one(north_one(position)) | west_one(north_one(position));
        attack_occupancy = occupancy.black;
        if(en_passant_square != -1) attack_occupancy = attack_occupancy | (1ULL << en_passant_square);
        vision = vision & attack_occupancy;
    } 
    if (color == BLACK) {
        vision = east_one(south_one(position)) | west_one(south_one(position));
        attack_occupancy = occupancy.white;
        if(en_passant_square != -1) attack_occupancy = attack_occupancy | (1ULL << en_passant_square);
        vision = vision & attack_occupancy;
    }
    return vision;
}

U64 get_pawn_move_vision(int square_number, Color color, Occupancy occupancy) {
    U64 position = 1ULL << square_number;
    U64 vision;
    if(color == WHITE) {
        vision = north_one(position) & occupancy.not_all;
        if((position & RANK_2) && vision) {
            vision = vision | (north_one(vision) & occupancy.not_all);
        }
    }
    if(color == BLACK) {
        vision = south_one(position) & occupancy.not_all;
        if((position & RANK_7) && vision) {
            vision = vision | (south_one(vision) & occupancy.not_all);
        }
    }
    return vision;
}

U64 get_piece_vision(int p, U64 occupied, PieceType piece_type) {

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
    return (alg[1] - '1') + 8 * (alg[0] - 'a');
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

std::ostream& operator<<(std::ostream& os, const Piece& piece) {
    os << (piece.color == 1 ? "White " : "Black ") << piece_to_char(piece) << " on square " << square_to_algebraic(piece.square_number) << '\n';
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
    os << "En passant square: " << state.en_passant_square << " | Fullmove clock: " << state.fullmove_number << " | Halfmove clock: " << state.halfmove_clock << '\n';
    return os;
}

Occupancy get_occupancy(vector<Piece>& pieces) {
    Occupancy occupancy;
    for(Piece piece : pieces) {
        occupancy.all = occupancy.all | (1ULL << piece.square_number);
        if(piece.color == WHITE) occupancy.white = occupancy.white | (1ULL << piece.square_number);
        if(piece.color == BLACK) occupancy.black = occupancy.black | (1ULL << piece.square_number);
    }
    occupancy.not_all = ~occupancy.all;
    return occupancy;
}

vector<int> get_set_bit_positions(U64 bb) {
    vector<int> positions;
    while(bb) {
        int i = count_zeros(bb);
        positions.push_back(i);
    }
}

PieceType lookup_piece_at_square(vector<Piece>& pieces, int square_number) {
    for (Piece piece : pieces) {
        if(square_number == piece.square_number) {
            return piece.type;
        }
    }
    return EMPTY;
}

vector<Move> get_all_legal_moves(GameState& state) {
    // Create occupancy bb
    vector<Move> legal_moves;
    Occupancy occupancy = get_occupancy(state.pieces);
    for(Piece piece : state.pieces) {
        if(piece.type == PAWN) {
            U64 move_set = get_pawn_move_vision(piece.square_number, piece.color, occupancy);
            U64 attack_set = get_pawn_attack_vision(piece.square_number, piece.color, occupancy, state.en_passant_square);
            
            // Promotions
            if((piece.square_number > 47 && piece.color == WHITE) || (piece.square_number < 16 && piece.color == BLACK)) {
                if(move_set) {
                    int promotion = get_set_bit_positions(move_set)[0];
                    for(PieceType piece_promotion : legal_promotions) {
                        legal_moves.push_back({piece.square_number, promotion, piece.type, EMPTY, piece_promotion, PROMOTION});
                    }
                }
                if(attack_set) {
                    vector<int> captures_and_promotions = get_set_bit_positions(attack_set);
                    for(int promotion : captures_and_promotions) {
                        for(PieceType piece_promotion : legal_promotions) {
                            PieceType captured_piece = lookup_piece_at_square(state.pieces, promotion);
                            legal_moves.push_back({piece.square_number, promotion, piece.type, captured_piece, piece_promotion, CAPTURE_AND_PROMOTION});
                        }
                    }
                }
            }
            // Pushes and Double Pushes
            if((piece.square_number > 47 && piece.color == BLACK) || (piece.square_number < 16 && piece.color == WHITE)) {
                if(move_set) {
                    vector<int> pushes = get_set_bit_positions(move_set);
                    if(pushes.size() > 1) {}
                    for(int push : pushes) {
                        if(abs(piece.square_number - push) == 16) {
                            legal_moves.push_back({piece.square_number, push, piece.type, EMPTY, EMPTY, DOUBLE_PUSH});
                        } else {
                            legal_moves.push_back({piece.square_number, push, piece.type, EMPTY, EMPTY, PUSH});
                        }
                    }
                }
            }
            // Captures, including en passant
            if(attack_set)
        }
    }
}
 