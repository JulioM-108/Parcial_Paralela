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

int main(){
      vector<vector<char>> board;
      string in;
      cin >> in;

      for (int r = 0; r < 4; ++r){
            vector<char> row;
            for (int i = 0; i < 4; i++) {
                  row.push_back(in[4 * r + i]);
            }
            board.push_back(row);
      }
      print_board(board);
      return 0;
}