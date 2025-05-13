#include "include/chess.hpp"

using std::cout;
using std::endl;

int main() {
    cout << "Testing" << endl;
    U64 fourth_rank_occupied = 0x00000000FF000000ULL;
    U64 e_file_occupied = 0x0808080808080808ULL;
    int count = 0;
    /*
    U64 board = 1ULL << 27;
    print_bitboard(board);
    Rook test(board, false, LIGHT);
    print_bitboard(test.rook_vision(fourth_rank_occupied));

    */
    for (int i = 0; i < 64; i++) {
        cout << endl;
        Rook test_rook(i, false, LIGHT);
        Bishop test_bishop(i, LIGHT);
        Queen test_queen(i, LIGHT);
        print_bitboard(test_rook.rook_vision(FILE_A));
        print_bitboard(test_bishop.bishop_vision(FILE_A));
        print_bitboard(test_queen.queen_vision(FILE_A));
        cout << ++count << endl;
    }


    cout << endl;
}