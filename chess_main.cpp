#include "include/chess.hpp"

int main() {
    cout << "Starting..." << endl;
    string mate_in_1 = "4k3/R6R/8/8/8/8/8/4K3 w - - 0 1";
    string mate_in_2 = "kbK5/pp6/1P6/8/8/8/8/R7 w - - 0 1";
    string mate_in_3 = "8/8/6pr/6p1/5pPk/5P1p/5P1K/R7 w - - 0 1";
    string hard_puzzle = "3r4/2R2pb1/4pk1p/6p1/4Q3/3P3P/Pr3qP1/5R1K w - - 5 26";
    GameState state = parse_full_fen(hard_puzzle);
    MoveEval selected = alpha_beta_search(state, 4);
    cout << selected.best_move << '\n' << selected.evaluation << endl;
    return -1;
}