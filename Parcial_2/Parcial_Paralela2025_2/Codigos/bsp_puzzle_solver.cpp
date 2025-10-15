#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_set>
#include <string>
#include <iomanip>
#include <ctime>

using namespace std;

// const int dRow[] = {-1, 1, 0, 0}; // UP, DOWN, LEFT, RIGHT
// const int dCol[] = {0, 0, -1, 1};
// const string MOVES[] = {"UP", "DOWN", "LEFT", "RIGHT"};

// struct State{
//       string board;
//       int blankPos;
//       int cost;
//       State(const string &b, int pos, int c) : board(b), blankPos(pos), cost(c) {}
// };

// // =============================================================================
// // HELPER FUNCTIONS
// // =============================================================================

// /**
//  * @brief Swaps two tiles on the board and returns new board state
//  * 
//  */
// string swapBoardTiles(const string &currentBoard, int position1, int position2){
//       string newBoard = currentBoard;
//       char tmp = newBoard[position1];
//       newBoard[position1] = newBoard[position2];
//       newBoard[position2] = tmp;
//       return newBoard;
// }

// /**
//  * @brief Breadth-First Search to find shortest path to goal state
//  * 
//  * Explores all possible states level by level, guaranteeing the shortest path
//  * will be found first due to BFS properties.
//  */
// int bfs(const string &start){
//       const string goal = "ABCDEFGHIJKLMNO#";

//       if (start.size() < 16) return -1;

//       queue<State> q;
//       unordered_set<string> visited;


//       size_t found = start.find('#');
//       if (found == string::npos) return -1;
//       int blankPos = static_cast<int>(found);

//       q.push(State(start, blankPos, 0));
//       visited.insert(start);

//       while (!q.empty()){
//             State current = q.front();
//             q.pop();

//             if (current.board == goal) return current.cost;
            
//             // TODO: complete the conditiional
//             // Check if goal state is reached
//             // If the goal state is reached, return the cost
//             // if (______________) return current.cost;

//             // TODO: Convert 1D position to 2D coordinates
//             // FIXME: Create an auxiliar function 
//             int row = current.blankPos / 4;
//             int col = current.blankPos % 4;

//             // Try all 4 possible moves
//             for (int i = 0; i < 4; i++){
//                   int newRow = row + dRow[i];
//                   int newCol = col + dCol[i];

//                   // Check if the move is within bounds
//                   if (newRow >= 0 && newRow < 4 && newCol >= 0 && newCol < 4){
//                         // TODO: Convert 2D coordinates back to 1D position
//                         int newPos = newRow * 4 + newCol;
//                         string newBoard = swapBoardTiles(current.board, current.blankPos, newPos);

//                         // If this state hasn't been visited, add it to the queue
//                         if (visited.find(newBoard) == visited.end()){
//                               visited.insert(newBoard);
//                               q.push(State(newBoard, newPos, current.cost + 1));     
//                         }
//                   }
//             }
//       }
//       // No solution found
//       return -1;
// }

// // =============================================================================
// // MAIN FUNCTION
// // =============================================================================

// int main(int argc, char** argv) {
//     string start;
  
//     if (argc >= 2) {
//         start = argv[1];
//     } else {
//         if (!(cin >> start)) return 0;
//     }

//     int result = bfs(start);

//     if (result == -1)
//         cout << "UNSOLVABLE\n";
//     else
//         cout << result << "\n";

//     return 0;
// }

//Modificación para la Tarea 10.

const int dRow[] = {-1, 1, 0, 0}; // UP, DOWN, LEFT, RIGHT
const int dCol[] = {0, 0, -1, 1};
const string GOAL = "ABCDEFGHIJKLMNO#";

struct State {
    string board;
    int blankPos;
    int cost;
    State(const string &b, int pos, int c) : board(b), blankPos(pos), cost(c) {}
};

// -----------------------------------------------------------------------------
// Swaps two tiles and returns a new board configuration
// -----------------------------------------------------------------------------
string swapBoardTiles(const string &currentBoard, int p1, int p2) {
    string newBoard = currentBoard;
    swap(newBoard[p1], newBoard[p2]);
    return newBoard;
}

// -----------------------------------------------------------------------------
// BFS search that counts expanded nodes
// -----------------------------------------------------------------------------
int bfs_search(const string &start, unsigned long long &expanded) {
    if (start.size() != 16) return -1;

    size_t found = start.find('#');
    if (found == string::npos) return -1;
    int blankPos = static_cast<int>(found);

    queue<State> q;
    unordered_set<string> visited;
    q.push(State(start, blankPos, 0));
    visited.insert(start);

    while (!q.empty()) {
        State current = q.front(); q.pop();
        ++expanded; // cada vez que sacamos un nodo de la cola, lo expandimos

        if (current.board == GOAL) return current.cost;

        int row = current.blankPos / 4;
        int col = current.blankPos % 4;

        for (int i = 0; i < 4; i++) {
            int newRow = row + dRow[i];
            int newCol = col + dCol[i];

            if (newRow >= 0 && newRow < 4 && newCol >= 0 && newCol < 4) {
                int newPos = newRow * 4 + newCol;
                string newBoard = swapBoardTiles(current.board, current.blankPos, newPos);

                if (visited.find(newBoard) == visited.end()) {
                    visited.insert(newBoard);
                    q.push(State(newBoard, newPos, current.cost + 1));
                }
            }
        }
    }

    return -1; // sin solución
}

// -----------------------------------------------------------------------------
// MAIN — lee puzzles.txt y muestra costo, nodos expandidos, tiempo
// -----------------------------------------------------------------------------
int main() {
    ifstream fin("puzzles.txt");
    if (!fin) {
        cerr << "No se pudo abrir puzzles.txt\n";
        return 1;
    }

    string puzzle;
    int caseNum = 1;

    cout << left << setw(6) << "Caso"
         << setw(20) << "Puzzle"
         << setw(10) << "Costo"
         << setw(18) << "Nodos Expandidos"
         << setw(15) << "Tiempo(s)" << endl;
    cout << string(70, '-') << endl;

    while (fin >> puzzle) {
        unsigned long long expanded = 0;

        clock_t t0 = clock();
        int cost = bfs_search(puzzle, expanded);
        clock_t t1 = clock();

        double secs = double(t1 - t0) / CLOCKS_PER_SEC;

        cout << setw(6) << caseNum
             << setw(20) << puzzle
             << setw(10) << cost
             << setw(18) << expanded
             << setw(15) << fixed << setprecision(6) << secs
             << endl;

        caseNum++;
    }

    fin.close();
    return 0;
}