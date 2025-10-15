

#include <bits/stdc++.h>
#include <atomic>
#include <ctime>
#include <iomanip>
#include <omp.h>
using namespace std;

int BOARD_SIZE = 4;
int TOTAL_TILES;
string TARGET;
bool USE_NUMBERS = false;

void generateTarget() {
    TOTAL_TILES = BOARD_SIZE * BOARD_SIZE;
    TARGET.clear();
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
        while (getline(ss, token, ',')) tokens.push_back(token);
    } else {
        USE_NUMBERS = false;
        for (char c : input) tokens.push_back(string(1, c));
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
        if (tokens[i] == "#") blankPos = int(i);
        else {
            if (USE_NUMBERS) values.push_back(stoi(tokens[i]));
            else values.push_back((int)tokens[i][0]);
        }
    }
    int inversions = 0;
    for (size_t i = 0; i < values.size(); ++i)
        for (size_t j = i + 1; j < values.size(); ++j)
            if (values[i] > values[j]) ++inversions;

    if (BOARD_SIZE % 2 == 1) return (inversions % 2) == 0;
    else {
        int rowFromBottom = BOARD_SIZE - (blankPos / BOARD_SIZE);
        return ((inversions + rowFromBottom) % 2) == 1;
    }
}

unordered_map<string, pair<int,int>> goalPositions;
void initGoalPositions() {
    goalPositions.clear();
    vector<string> t = parseState(TARGET);
    for (size_t i = 0; i < t.size(); ++i)
        goalPositions[t[i]] = {int(i / BOARD_SIZE), int(i % BOARD_SIZE)};
}

int heuristic_h2(const vector<string> &tokens) {
    int sum = 0;
    for (size_t i = 0; i < tokens.size(); ++i) {
        string tile = tokens[i];
        if (tile == "#") continue;
        auto it = goalPositions.find(tile);
        if (it == goalPositions.end()) continue;
        auto [gr, gc] = it->second;
        int r = int(i / BOARD_SIZE);
        int c = int(i % BOARD_SIZE);
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


atomic<int> bestSolution; 


void aStarThread(const string &startStr, int gStart) {
    if (bestSolution.load() <= gStart) return;

    unordered_map<string,int> bestG;
    priority_queue<Node, vector<Node>, Cmp> open;

    vector<string> startTokens = parseState(startStr);
    int h0 = heuristic_h2(startTokens);
    open.emplace(h0 + gStart, gStart, startStr);
    bestG[startStr] = gStart;

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    while (!open.empty()) {
        Node cur = open.top(); open.pop();

       
        int globalBest = bestSolution.load();
        if (cur.g >= globalBest) continue;

        auto it = bestG.find(cur.state);
        if (it != bestG.end() && cur.g > it->second) continue;

        if (cur.state == TARGET) {
            int old = bestSolution.load();
            while (cur.g < old && !bestSolution.compare_exchange_weak(old, cur.g)) {
                
            }
            return;
        }

        vector<string> tokens = parseState(cur.state);
        int pos = -1;
        for (size_t i = 0; i < tokens.size(); ++i) if (tokens[i] == "#") { pos = int(i); break; }
        int r = pos / BOARD_SIZE;
        int c = pos % BOARD_SIZE;

        for (int k = 0; k < 4; ++k) {
            int nr = r + dr[k], nc = c + dc[k];
            if (nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE) continue;
            int npos = nr * BOARD_SIZE + nc;
            vector<string> newTokens = tokens;
            swap(newTokens[pos], newTokens[npos]);
            string t = tokensToString(newTokens);
            int ng = cur.g + 1;

            if (ng >= bestSolution.load()) continue;

            auto it2 = bestG.find(t);
            if (it2 == bestG.end() || ng < it2->second) {
                bestG[t] = ng;
                int hf = heuristic_h2(newTokens);
                int f = ng + hf;
                if (f >= bestSolution.load()) continue;
                open.emplace(f, ng, t);
            }
        }
    }
}


int parallelAStar(const string &startStr) {
    vector<string> startTokens = parseState(startStr);
    int n = (int)startTokens.size();
    BOARD_SIZE = (int) sqrt(n);
    if (BOARD_SIZE * BOARD_SIZE != n) {
        cerr << "Error: El número de elementos no forma un tablero cuadrado\n";
        return -1;
    }

    generateTarget();
    if (startStr == TARGET) return 0;
    if (!isSolvable(startTokens)) return -1;
    initGoalPositions();

    int pos = -1;
    for (size_t i = 0; i < startTokens.size(); ++i) if (startTokens[i] == "#") { pos = int(i); break; }
    int r = pos / BOARD_SIZE, c = pos % BOARD_SIZE;
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    vector<string> children;
    for (int k = 0; k < 4; ++k) {
        int nr = r + dr[k], nc = c + dc[k];
        if (nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE) continue;
        int npos = nr * BOARD_SIZE + nc;
        vector<string> newTokens = startTokens;
        swap(newTokens[pos], newTokens[npos]);
        children.push_back(tokensToString(newTokens));
    }

    if (children.empty()) return -1;

   
    bestSolution.store(INT_MAX);



    int m = (int)children.size();

 
    #pragma omp parallel for default(none) shared(children, m) schedule(dynamic)
    for (int i = 0; i < m; ++i) {
       
        aStarThread(children[i], 1);
    }

    int ans = bestSolution.load();
    if (ans == INT_MAX) return -1;
    return ans;
}

int main() {
    cout << "Ingrese el estado inicial del puzzle:\n";
    cout << "Para letras (4x4): ABCDEFGHIJKLMNO#\n";
    cout << "Para números: 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,#\n";
    

    string input;
    if (!(cin >> input)) return 0;

    clock_t t0 = clock();
    int result = parallelAStar(input);
    clock_t t1 = clock();

    double secs = double(t1 - t0) / CLOCKS_PER_SEC;

    if (result == -1) cout << "UNSOLVABLE\n";
    else cout << result << "\n";

    cout << "Tiempo (clock): " << fixed << setprecision(6) << secs << " s\n";
    return 0;
}
