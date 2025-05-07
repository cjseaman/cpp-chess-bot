#include "include/chess.hpp"

using std::cout;
using std::endl;

int main() {
    cout << "Testing Pawns" << endl;
    int count = 0;
    for (U64 board : position_in_each_square()) {
        Pawn test_pawn(board, false, DARK);
        cout << endl;
        print_bitboard(test_pawn.p_move_vision() | test_pawn.p_attack_vision());
        cout << ++count << endl;
    }
    cout << endl;
}