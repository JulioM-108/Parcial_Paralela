#include <iostream>
#include <string>

using namespace std;


void listAvailable(const string &board){
    if (board.size() < 16){
      return;
    }
    
    size_t pos = board.find('#');
    if (pos == string::npos) {
      return; 
    }
    int row = pos / 4;
    int col = pos % 4;

    if (row > 0)  cout << "UP" << '\n';
    if (row < 3)  cout << "DOWN" << '\n';
    if (col > 0)  cout << "LEFT" << '\n';
    if (col < 3)  cout << "RIGHT" << '\n';
}

int main(int argc, char** argv) {
    string board;
    if (argc >= 2){
        board = argv[1];
    }
    else{
        cin >> board;
    }
    if (board.size() > 16){
        board = board.substr(0, 16);
    }
    else if (board.size() < 16){
        board += string(16 - board.size(), ' ');
    }
    listAvailable(board);
}