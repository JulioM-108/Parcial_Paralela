/**
 * @file h1_ puzzle_solver.cpp
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
#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>

using namespace std;
// const string TARGET = "ABCDEFGHIJKLMNO#";

// bool isSolvable(const string &s){
//     vector<int> tiles;
//     for(char c : s){
//         if(c != '#') {
//             tiles.push_back((int)c);
//         } 
//     }
//     int inv = 0;
//     for(size_t i=0;i<tiles.size();++i)
//         for(size_t j=i+1;j<tiles.size();++j)
//             if(tiles[i] > tiles[j]) {
//                   ++inv;
//             }

 
//     int blankIdx = s.find('#'); 
//     int rowFromTop = blankIdx / 4 + 1; 
//     int rowFromBottom = 4 - (rowFromTop - 1); 
 
//     return ((inv + rowFromBottom) % 2) == 1;
// }

// int heuristic_h1(const string &state){
//     int cnt = 0;
//     for(int i=0;i<16;++i){
//         if(state[i] == '#') {
//             continue;
//         }
//         if(state[i] != TARGET[i]){
//             ++cnt;
//         } 
//     }
//     return cnt;
// }

// struct Node {
//     int f;
//     int g;
//     string state;
//     Node(int _f,int _g,const string &_s):f(_f),g(_g),state(_s){}
// };

// struct Cmp {
//     bool operator()(Node const &a, Node const &b) const {
//         if(a.f != b.f) {
//             return a.f > b.f;
//         }
//         else{
//             return a.g < b.g; 
//         }
        
//     }
// };

// int aStarSearch(string start){
//     if(start == TARGET) {
//       return 0;
//     }
//     if(!isSolvable(start)) {
//       return -1;
//     }

//     priority_queue<Node, vector<Node>, Cmp> open;
//     unordered_map<string,int> bestG; 

//     int h0 = heuristic_h1(start);
//     open.emplace(h0, 0, start);
//     bestG[start] = 0;

   
//     const int dr[4] = {-1,1,0,0}; // UP, DOWN, LEFT, RIGHT
//     const int dc[4] = {0,0,-1,1};

//     while(!open.empty()){
//         Node cur = open.top(); open.pop();
//         string s = cur.state;
//         int g = cur.g;

//         auto it = bestG.find(s);
//         if(it != bestG.end() && g > it->second) {
//             continue;
//         }

//         if(s == TARGET) {
//             return g;
//         }

//         int pos = s.find('#');
//         int r = pos / 4;
//         int c = pos % 4;

//         for(int k=0;k<4;++k){
//             int nr = r + dr[k];
//             int nc = c + dc[k];
//             if(nr < 0 || nr >= 4 || nc < 0 || nc >= 4) {
//                   continue;
//             }
//             int npos = nr*4 + nc;
//             string t = s;
//             swap(t[pos], t[npos]); 
//             int ng = g + 1;
//             auto it2 = bestG.find(t);
//             if(it2 == bestG.end() || ng < it2->second){
//                 bestG[t] = ng;
//                 int hf = heuristic_h1(t);
//                 int f = ng + hf;
//                 open.emplace(f, ng, t);
//             }
//         }
//     }

//     return -1;
// }


// int main(int argc, char* argv[]){
//     string start;
//     if(argc > 1){
//         start = string(argv[1]);
//     } else {
//         if(!(cin >> start)) return 0;
//     }

//     int result = aStarSearch(start);
//     cout << result << endl;
//     return 0;
// }

// Modifiación para la Tarea 10.


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

int heuristic_h1(const string &state){
    int cnt = 0;
    for(int i=0;i<16;++i){
        if(state[i] == '#') continue;
        if(state[i] != TARGET[i]) ++cnt;
    }
    return cnt;
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


int aStarSearch(string start, unsigned long long &expanded){
    if(start == TARGET) return 0;
    if(!isSolvable(start)) return -1;

    priority_queue<Node, vector<Node>, Cmp> open;
    unordered_map<string,int> bestG;

    int h0 = heuristic_h1(start);
    open.emplace(h0, 0, start);
    bestG[start] = 0;

    const int dr[4] = {-1,1,0,0};
    const int dc[4] = {0,0,-1,1};

    while(!open.empty()){
        Node cur = open.top(); open.pop();
        ++expanded; 

        string s = cur.state;
        int g = cur.g;

        auto it = bestG.find(s);
        if(it != bestG.end() && g > it->second) continue;

        if(s == TARGET) return g;

        int pos = s.find('#');
        int r = pos / 4;
        int c = pos % 4;

        for(int k=0;k<4;++k){
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
                int hf = heuristic_h1(t);
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

    string puzzle;
    int caseNum = 1;

    cout << left << setw(8) << "Caso" 
         << setw(20) << "Puzzle"
         << setw(15) << "Costo"
         << setw(20) << "Nodos Expandidos" << endl;
    cout << string(65, '-') << endl;

    while(fin >> puzzle){
        unsigned long long expanded = 0;
        int cost = aStarSearch(puzzle, expanded);
        cout << setw(8) << caseNum 
             << setw(20) << puzzle 
             << setw(15) << cost 
             << setw(20) << expanded 
             << endl;
        caseNum++;
    }

    fin.close();
    return 0;
}
