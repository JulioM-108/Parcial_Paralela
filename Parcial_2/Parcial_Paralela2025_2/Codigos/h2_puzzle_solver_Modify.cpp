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
#include <queue>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <sstream>
using namespace std;

int BOARD_SIZE = 4;
int TOTAL_TILES;
string TARGET;
bool USE_NUMBERS = false;

void generateTarget() {
    TOTAL_TILES = BOARD_SIZE * BOARD_SIZE;
    TARGET = "";
    
    if (USE_NUMBERS) {
        for (int i = 1; i < TOTAL_TILES; ++i) {
            TARGET += to_string(i);
            if (i < TOTAL_TILES - 1) TARGET += ",";
        }
        TARGET += ",#";
    } else {
        for (int i = 0; i < TOTAL_TILES - 1; ++i) {
            if (i < 26) {
                TARGET += char('A' + i);
            } else {
                int first = (i / 26) - 1;
                int second = i % 26;
                if (first >= 0) TARGET += char('A' + first);
                TARGET += char('A' + second);
            }
        }
        TARGET += '#';
    }
}

vector<string> parseState(const string &input) {
    vector<string> tokens;
    if (input.find(',') != string::npos) {
        USE_NUMBERS = true;
        stringstream ss(input);
        string token;
        while (getline(ss, token, ',')) {
            tokens.push_back(token);
        }
    } else {
        USE_NUMBERS = false;
        for (char c : input) {
            tokens.push_back(string(1, c));
        }
    }
    return tokens;
}

string tokensToString(const vector<string> &tokens) {
    string result;
    for (size_t i = 0; i < tokens.size(); ++i) {
        result += tokens[i];
        if (USE_NUMBERS && i < tokens.size() - 1) result += ",";
    }
    return result;
}

bool isSolvable(const vector<string> &tokens) {
    vector<int> values;
    int blankPos = -1;
    
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == "#") {
            blankPos = i;
        } else {
            if (USE_NUMBERS) {
                values.push_back(stoi(tokens[i]));
            } else {
                values.push_back((int)tokens[i][0]);
            }
        }
    }
    
    int inversions = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        for (size_t j = i + 1; j < values.size(); ++j) {
            if (values[i] > values[j]) ++inversions;
        }
    }
    
    if (BOARD_SIZE % 2 == 1) {
        return (inversions % 2) == 0;
    } else {
        int rowFromBottom = BOARD_SIZE - (blankPos / BOARD_SIZE);
        return ((inversions + rowFromBottom) % 2) == 1;
    }
}

unordered_map<string, pair<int,int>> goalPositions;

void initGoalPositions() {
    goalPositions.clear();
    vector<string> targetTokens = parseState(TARGET);
    for (size_t i = 0; i < targetTokens.size(); ++i) {
        goalPositions[targetTokens[i]] = {i / BOARD_SIZE, i % BOARD_SIZE};
    }
}


int heuristic_h2(const vector<string> &tokens) {
    int sum = 0;
    for (size_t i = 0; i < tokens.size(); ++i) {
        string tile = tokens[i];
        if (tile == "#") continue;
        
        auto it = goalPositions.find(tile);
        if (it == goalPositions.end()) continue;
        
        auto [gr, gc] = it->second;
        int r = i / BOARD_SIZE;
        int c = i % BOARD_SIZE;
        sum += abs(r - gr) + abs(c - gc);
    }
    return sum;
}

struct Node {
    int f;
    int g;
    string state;
    Node(int _f, int _g, const string &_s) : f(_f), g(_g), state(_s) {}
};

struct Cmp {
    bool operator()(Node const &a, Node const &b) const {
        if (a.f != b.f) return a.f > b.f;
        return a.g < b.g;
    }
};

int aStarSearch(const string &startStr) {
    vector<string> startTokens = parseState(startStr);
    
   
    int n = startTokens.size();
    BOARD_SIZE = (int)sqrt(n);
    
    if (BOARD_SIZE * BOARD_SIZE != n) {
        cerr << "Error: El número de elementos no forma un tablero cuadrado" << endl;
        return -1;
    }
    
    generateTarget();
    
    if (startStr == TARGET) return 0;
    if (!isSolvable(startTokens)) return -1;
    
    initGoalPositions();
    
    priority_queue<Node, vector<Node>, Cmp> open;
    unordered_map<string, int> bestG;
    
    int h0 = heuristic_h2(startTokens);
    open.emplace(h0, 0, startStr);
    bestG[startStr] = 0;
    
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};
    
    while (!open.empty()) {
        Node cur = open.top();
        open.pop();
        
        string s = cur.state;
        int g = cur.g;
        
        auto it = bestG.find(s);
        if (it != bestG.end() && g > it->second) continue;
        
        if (s == TARGET) return g;
        
        vector<string> tokens = parseState(s);
        
        
        int pos = -1;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (tokens[i] == "#") {
                pos = i;
                break;
            }
        }
        
        int r = pos / BOARD_SIZE;
        int c = pos % BOARD_SIZE;
        
        
        for (int k = 0; k < 4; ++k) {
            int nr = r + dr[k];
            int nc = c + dc[k];
            
            if (nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE) continue;
            
            int npos = nr * BOARD_SIZE + nc;
            vector<string> newTokens = tokens;
            swap(newTokens[pos], newTokens[npos]);
            string t = tokensToString(newTokens);
            
            int ng = g + 1;
            auto it2 = bestG.find(t);
            
            if (it2 == bestG.end() || ng < it2->second) {
                bestG[t] = ng;
                int hf = heuristic_h2(newTokens);
                int f = ng + hf;
                open.emplace(f, ng, t);
            }
        }
    }
    
    return -1;
}

int main() {
    string input;
    
    cout << "Ingrese el estado inicial del puzzle:" << endl;
    cout << "Para letras (4x4): ABCDEFGHIJKLMNO#" << endl;
    cout << "Para números: 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,#" << endl;
    
    if (!(cin >> input)) return 0;
    
    int result = aStarSearch(input);
    
    if (result == -1) {
        cout << "UNSOLVABLE" << endl;
    } else {
        cout << result << endl;
    }
    
    return 0;
}