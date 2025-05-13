#include "../include/chess.hpp"

using namespace std;

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
    vector<U64> all_board_positions = position_in_each_square();
    int position_index = 0;

    for(U64 position : all_board_positions) {

        position_index = posi(position);
        cout << position_index << " ";

        U64 rook_rays = draw_rook_rays(position);
        U64 bishop_rays = draw_bishop_rays(position);
        U64 outer_squares = FILE_A | FILE_H | RANK_1 | RANK_8;

        piece_bnb[EMPTY][position_index] = position;
        piece_bnb[KING][position_index] = 0;
        piece_bnb[ROOK][position_index] = rook_rays & ~outer_squares;
        piece_bnb[BISHOP][position_index] = bishop_rays & ~outer_squares;
        piece_bnb[KNIGHT][position_index] = 0;
        piece_bnb[QUEEN][position_index] = piece_bnb[BISHOP][position_index] | piece_bnb[ROOK][position_index];

        piece_vision[EMPTY][position_index] = position;
        piece_vision[KING][position_index] = draw_king_vision(position);
        piece_vision[ROOK][position_index] = rook_rays;
        piece_vision[KNIGHT][position_index] = draw_knight_vision(position);
        piece_vision[BISHOP][position_index] = bishop_rays;
        piece_vision[QUEEN][position_index] = piece_vision[BISHOP][position_index] | piece_vision[ROOK][position_index];
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

U64 p_attack_vision(int square_number, Color color) {
    U64 position = 1ULL << square_number;
    U64 vision;
    if (color == LIGHT) {
        vision = east_one(north_one(position)) | west_one(north_one(position));
    } 
    if (color == DARK) {
        vision = east_one(south_one(position)) | west_one(south_one(position));
    }
    return vision;
}

U64 p_move_vision(int square_number, Color color) {
    U64 position = 1ULL << square_number;
    U64 vision;
    if(color == LIGHT) {
        vision = north_one(position);
        if(position & RANK_2) {
            vision = north_one(vision) | north_one(position);
        }
    }
    if(color == DARK) {
        vision = south_one(position);
        if(position & RANK_7) {
            vision = south_one(vision) | south_one(position);
        }
    }
    return vision;
}

U64 blocked_piece_vision(int p, U64 occupied, SquareOccupant piece_type) {
    U64 vision = piece_vision[piece_type][p];
    for(U64 b = occupied & piece_vision[piece_type][p]; b != 0; b = b & (b - 1)) {
        int sq = bit_scan_forward(b);
        vision = vision & ~behind[p][sq];
    }
    return vision;
}

SquareOccupant fen_to_piece(char piece_char) {
    piece_char = tolower(piece_char);
    switch(piece_char) {
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
    }
}

Game::Game(string start_position_fen) {
    
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

 