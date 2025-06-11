chess_board = Chessboard("chess_board", {position: "start", draggable: true, onChange: handle_move})
playing_as = "white"
selected_depth = 3

$("#submit_fen_import").click(function() {
    const fen_string_val = $("#fen_import").val()
    chess_board.position(fen_string_val, false)
})

$("#reset_game").click(function() {
    console.log("Resetting game")
    chess_board.start()
})

$("#play_as").change(function() {
    playing_as = $("#play_as").val()
    console.log("Playing as " + $("#play_as").val())
})

function handle_move(oldPosition, newPosition) {
    console.log("Handling Move")
    console.log(oldPosition)
    console.log(newPosition)
    fen_string = Chessboard.objToFen(newPosition)
    console.log(fen_string)
    $.get("/best_move", {"fen": fen_string, "depth": selected_depth}, "json").done(function(data) {
        console.log(data)
    })
}


