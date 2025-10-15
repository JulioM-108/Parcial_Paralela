/**
 * @file h2_ puzzle_solver.cpp
 * @brief 4x4 Sliding Puzzle Solver using BFS (Breadth-First Search)
 * 
 * This program solves the 4x4 sliding puzzle by finding the minimum number
 * of moves required to reach the goal state from the initial state.
 * 
 * Goal state: "ABCDEFGHIJKLMNO#"
 * Where '#' represents the empty space.
 * 
 * @author JAPeTo
 * @version 1.6
 */
/**
 * @file h2_puzzle_solver.cpp
 * @brief 4x4 Sliding Puzzle Solver using A* with h2 (Manhattan distance)
 *
 * Goal state: "ABCDEFGHIJKLMNO#"
 *
 * Author: JAPeTo (modificado)
 * Version: 1.6 -> + A* h2
 */
#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <array>
#include <tuple>
#include <ctime>
#include <iomanip>

using namespace std;

// const string TARGET = "ABCDEFGHIJKLMNO#";

// bool isSolvable(const string &s){
    
//     vector<int> tiles;
//     for(char c : s){
//         if(c != '#') tiles.push_back((int)c);
//     }
//     int inv = 0;
//     for(size_t i=0;i<tiles.size();++i)
//         for(size_t j=i+1;j<tiles.size();++j)
//             if(tiles[i] > tiles[j]) ++inv;

   
//     int blankIdx = s.find('#'); 
//     int rowFromTop = blankIdx / 4 + 1; 
//     int rowFromBottom = 4 - (rowFromTop - 1); 
//     return ((inv + rowFromBottom) % 2) == 1;
// }


// array<pair<int,int>, 256> goalPos; 

// void initGoalPositions(){
//     for(int i = 0; i < 16; ++i){
//         char c = TARGET[i];
//         goalPos[(unsigned char)c] = {i/4, i%4};
//     }
// }


// int heuristic_h2(const string &state){
//     int sum = 0;
//     for(int i = 0; i < 16; ++i){
//         char c = state[i];
//         if(c == '#') continue;
//         auto [gr, gc] = goalPos[(unsigned char)c];
//         int r = i / 4;
//         int col = i % 4;
//         sum += abs(r - gr) + abs(col - gc);
//     }
//     return sum;
// }

// struct Node {
//     int f;
//     int g;
//     string state;
//     Node(int _f,int _g,const string &_s):f(_f),g(_g),state(_s){}
// };

// struct Cmp {
//     bool operator()(Node const &a, Node const &b) const {
//         if(a.f != b.f) return a.f > b.f;
//         return a.g < b.g; 
//     }
// };

// int aStarSearch(string start){
//     if(start == TARGET) return 0;
//     if(!isSolvable(start)) return -1;

//     initGoalPositions();

//     priority_queue<Node, vector<Node>, Cmp> open;
//     unordered_map<string,int> bestG;

//     int h0 = heuristic_h2(start);
//     open.emplace(h0, 0, start);
//     bestG[start] = 0;

//     const int dr[4] = {-1,1,0,0};
//     const int dc[4] = {0,0,-1,1};

//     while(!open.empty()){
//         Node cur = open.top(); open.pop();
//         string s = cur.state;
//         int g = cur.g;

//         auto it = bestG.find(s);
//         if(it != bestG.end() && g > it->second) continue;

//         if(s == TARGET) return g;

//         int pos = s.find('#');
//         int r = pos / 4;
//         int c = pos % 4;

//         for(int k = 0; k < 4; ++k){
//             int nr = r + dr[k];
//             int nc = c + dc[k];
//             if(nr < 0 || nr >= 4 || nc < 0 || nc >= 4) continue;
//             int npos = nr*4 + nc;
//             string t = s;
//             swap(t[pos], t[npos]); 
//             int ng = g + 1;
//             auto it2 = bestG.find(t);
//             if(it2 == bestG.end() || ng < it2->second){
//                 bestG[t] = ng;
//                 int hf = heuristic_h2(t);
//                 int f = ng + hf;
//                 open.emplace(f, ng, t);
//             }
//         }
//     }

//     return -1;
// }

// int main(){
//     string start;
//     if(!(cin >> start)) return 0;

//     time_t t_start = time(nullptr);
//     char *tstart_str = ctime(&t_start);

//     clock_t c0 = clock();
//     int result = aStarSearch(start);
//     clock_t c1 = clock();

//     time_t t_end = time(nullptr);
//     char *tend_str = ctime(&t_end);

//     double secs = double(c1 - c0) / CLOCKS_PER_SEC;

//     if(result == -1) cout << "UNSOLVABLE\n";
//     else cout << result << "\n";

//     cout << "Tiempo (clock): " << fixed << setprecision(6) << secs << " s\n";


//     return 0;
// }

// Modificación para la Tarea 10.

const string TARGET = "ABCDEFGHIJKLMNO#";

bool isSolvable(const string &s){
    vector<int> tiles;
    for(char c : s){
        if(c != '#') tiles.push_back((int)c);
    }
    int inv = 0;
    for(size_t i=0;i<tiles.size();++i)
        for(size_t j=i+1;j<tiles.size();++j)
            if(tiles[i] > tiles[j]) ++inv;

    int blankIdx = s.find('#');
    int rowFromTop = blankIdx / 4 + 1;
    int rowFromBottom = 4 - (rowFromTop - 1);
    return ((inv + rowFromBottom) % 2) == 1;
}

array<pair<int,int>, 256> goalPos;

void initGoalPositions(){
    for(int i = 0; i < 16; ++i){
        char c = TARGET[i];
        goalPos[(unsigned char)c] = {i/4, i%4};
    }
}

int heuristic_h2(const string &state){
    int sum = 0;
    for(int i = 0; i < 16; ++i){
        char c = state[i];
        if(c == '#') continue;
        auto [gr, gc] = goalPos[(unsigned char)c];
        int r = i / 4;
        int col = i % 4;
        sum += abs(r - gr) + abs(col - gc);
    }
    return sum;
}

struct Node {
    int f;
    int g;
    string state;
    Node(int _f,int _g,const string &_s):f(_f),g(_g),state(_s){}
};

struct Cmp {
    bool operator()(Node const &a, Node const &b) const {
        if(a.f != b.f) return a.f > b.f;
        return a.g < b.g;
    }
};

// A* h2 que cuenta nodos expandidos (cada vez que sacamos de la frontera)
int aStarSearch_count(const string &start, unsigned long long &expanded){
    if(start == TARGET) return 0;
    if(!isSolvable(start)) return -1;

    priority_queue<Node, vector<Node>, Cmp> open;
    unordered_map<string,int> bestG;

    int h0 = heuristic_h2(start);
    open.emplace(h0, 0, start);
    bestG[start] = 0;

    const int dr[4] = {-1,1,0,0};
    const int dc[4] = {0,0,-1,1};

    while(!open.empty()){
        Node cur = open.top(); open.pop();
        ++expanded; // contamos la expansión al sacar de la frontera

        string s = cur.state;
        int g = cur.g;

        auto it = bestG.find(s);
        if(it != bestG.end() && g > it->second) continue;

        if(s == TARGET) return g;

        int pos = s.find('#');
        int r = pos / 4;
        int c = pos % 4;

        for(int k = 0; k < 4; ++k){
            int nr = r + dr[k];
            int nc = c + dc[k];
            if(nr < 0 || nr >= 4 || nc < 0 || nc >= 4) continue;
            int npos = nr*4 + nc;
            string t = s;
            swap(t[pos], t[npos]);
            int ng = g + 1;
            auto it2 = bestG.find(t);
            if(it2 == bestG.end() || ng < it2->second){
                bestG[t] = ng;
                int hf = heuristic_h2(t);
                int f = ng + hf;
                open.emplace(f, ng, t);
            }
        }
    }

    return -1;
}

int main(){
    ifstream fin("puzzles.txt");
    if(!fin){
        cerr << "No se pudo abrir puzzles.txt\n";
        return 1;
    }

    initGoalPositions();

    string puzzle;
    int caseNum = 1;

    cout << left << setw(6) << "Caso"
         << setw(20) << "Puzzle"
         << setw(10) << "Costo"
         << setw(18) << "Nodos Expandidos"
         << setw(15) << "Tiempo(s)" << endl;
    cout << string(70, '-') << endl;

    while(fin >> puzzle){
        unsigned long long expanded = 0;

        clock_t c0 = clock();
        int cost = aStarSearch_count(puzzle, expanded);
        clock_t c1 = clock();
        double secs = double(c1 - c0) / CLOCKS_PER_SEC;

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