#include <iostream>
#include <vector>

using namespace std;

void print_board(vector<vector<char>> board){
      for (int i = 0; i < 4; i++){
            for (int j = 0; j < 4; j++){
                  cout << board[i][j];
                  if (j < 3) cout << " ";
            }
            cout << "\n";
            if (i < 3) cout << endl;
      }
}


void doMove(vector<vector<char>> &board, string move){
      for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
          if (board[i][j] == '#'){
            if (move == "DOWN" && i < 3){
              char temp = board[i + 1][j];
              board[i][j] = temp;
              board[i + 1][j] = '#';
              break;
            }else if (move == "UP" && i > 0){
              char temp = board[i - 1][j];
              board[i][j] = temp;
              board[i - 1][j] = '#';
              break;
            }else if (move == "RIGHT" && j < 3){
              char temp = board[i][j + 1];
              board[i][j] = temp;
              board[i][j + 1] = '#';
              break;
            }else if (move == "LEFT" && j > 0){
              char temp = board[i][j - 1];
              board[i][j] = temp;
              board[i][j - 1] = '#';
              break;
            }
          }
        }
      }
      print_board(board);
}

int main(int argc, char** argv) {
    string in, move;

    if (argc >= 3) {
        in = argv[1];
        move = argv[2];
    } else {
        if (!(cin >> in)) return 0;
        if (!(cin >> move)) return 0;
    }

   
    if (in.size() > 16) in = in.substr(0, 16);
    else if (in.size() < 16) in += string(16 - in.size(), ' ');

   
    for (auto &c : move) c = static_cast<char>(toupper((unsigned char)c));

    vector<vector<char>> board(4, vector<char>(4));
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            board[r][c] = in[4 * r + c];
        }
    }

    doMove(board, move);
    return 0;
}