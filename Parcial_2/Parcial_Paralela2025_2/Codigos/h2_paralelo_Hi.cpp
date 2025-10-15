
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


static vector<unsigned long long> nodesExpandedPerThread;


void aStarThread(const string &startStr, int gStart) {
    int tid = 0;
    #ifdef _OPENMP
    tid = omp_get_thread_num();
    #endif

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

        
        nodesExpandedPerThread[tid]++;

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


struct Subproblem {
    string state;
    int g;
    Subproblem(const string &s, int gg): state(s), g(gg) {}
};

vector<Subproblem> generateSubproblems(const string &startStr, int d_split) {
    vector<Subproblem> subs;
    struct S { string state; int g; };
    deque<S> q;
    q.push_back({startStr, 0});
    while (!q.empty()) {
        S cur = q.front(); q.pop_front();
        if (cur.g == d_split) {
            subs.emplace_back(cur.state, cur.g);
            continue;
        }
        vector<string> tokens = parseState(cur.state);
        int pos = -1;
        for (size_t i = 0; i < tokens.size(); ++i) if (tokens[i] == "#") { pos = int(i); break; }
        int r = pos / BOARD_SIZE, c = pos % BOARD_SIZE;
        const int dr[4] = {-1,1,0,0}; const int dc[4] = {0,0,-1,1};
        for (int k = 0; k < 4; ++k) {
            int nr = r + dr[k], nc = c + dc[k];
            if (nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE) continue;
            int npos = nr * BOARD_SIZE + nc;
            vector<string> newTokens = tokens;
            swap(newTokens[pos], newTokens[npos]);
            string t = tokensToString(newTokens);
            q.push_back({t, cur.g + 1});
        }
    }
    
    if (subs.empty()) subs.emplace_back(startStr, 0);
    return subs;
}


int parallelAStar_hybrid(const string &startStr, int nThreads = 0, int d_split = 2) {
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

    
    vector<Subproblem> subs = generateSubproblems(startStr, d_split);

    
    bestSolution.store(INT_MAX);

    if (nThreads > 0) omp_set_num_threads(nThreads);
    int threadsToUse = 1;
    #ifdef _OPENMP
    threadsToUse = omp_get_max_threads();
    #endif
    nodesExpandedPerThread.assign(threadsToUse, 0ULL);

    int m = (int)subs.size();

    
    #pragma omp parallel for default(none) shared(subs, m) schedule(dynamic)
    for (int i = 0; i < m; ++i) {
        aStarThread(subs[i].state, subs[i].g);
    }

    int ans = bestSolution.load();
    if (ans == INT_MAX) return -1;
    return ans;
}

// int main(int argc, char** argv) {
//     cout << "Ingrese el estado inicial del puzzle:\n";
//     cout << "Para letras (4x4): ABCDEFGHIJKLMNO#\n";
//     cout << "Para numeros: 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,#\n";

 
//     int nThreads = 0;
//     int d_split = 2;
//     if (argc >= 2) {
//         try { nThreads = stoi(argv[1]); }
//         catch (...) { nThreads = 0; }
//         if (nThreads < 1) nThreads = 0;
//     }
//     if (argc >= 3) {
//         try { d_split = stoi(argv[2]); }
//         catch (...) { d_split = 2; }
//         if (d_split < 0) d_split = 0;
//     }

//     string input;
//     if (!(cin >> input)) return 0;

//     time_t t_start_time = time(nullptr);
//     char *tstart_str = ctime(&t_start_time);

//     clock_t c0 = clock();
//     int result = parallelAStar_hybrid(input, nThreads, d_split);
//     clock_t c1 = clock();

//     time_t t_end_time = time(nullptr);
//     char *tend_str = ctime(&t_end_time);

//     double secs = double(c1 - c0) / CLOCKS_PER_SEC;

//     if (result == -1) cout << "UNSOLVABLE\n";
//     else cout << result << "\n";

//     cout << "Tiempo (clock): " << fixed << setprecision(6) << secs << " s\n";

   
//     unsigned long long totalNodes = 0;
//     for (size_t i = 0; i < nodesExpandedPerThread.size(); ++i) totalNodes += nodesExpandedPerThread[i];

//     cout << "Nodos expandidos totales: " << totalNodes << "\n";
//     for (size_t i = 0; i < nodesExpandedPerThread.size(); ++i) {
//         cout << "  Hilo " << i << ": " << nodesExpandedPerThread[i] << " nodos\n";
//     }

//     cout << "Parametros usados: nThreads=" 
//          << (nThreads>0? to_string(nThreads) : string("OMP_DEFAULT(")+to_string(omp_get_max_threads())+")")
//          << ", d_split=" << d_split << "\n";

//     return 0;
// }

// Modificación para Realizar el Punto 11.

int main(int argc, char** argv) {
    ifstream fin("puzzles.txt");
    if (!fin) {
        cerr << "No se pudo abrir puzzles.txt\n";
        return 1;
    }

    int nThreads = 4;   // Valor por defecto
    int d_split = 2;    // Profundidad de descomposición por defecto

    if (argc >= 2) nThreads = stoi(argv[1]);
    if (argc >= 3) d_split = stoi(argv[2]);

    omp_set_num_threads(nThreads);

    string puzzle;
    int caseNum = 1;

    cout << left << setw(6) << "Caso"
         << setw(20) << "Puzzle"
         << setw(10) << "Costo"
         << setw(18) << "Nodos Totales"
         << setw(25) << "Nodos por hilo"
         << setw(15) << "Tiempo(s)"
         << setw(12) << "Hilos" << endl;
    cout << string(120, '-') << endl;

    while (fin >> puzzle) {
        // Reiniciar contadores de nodos
        nodesExpandedPerThread.assign(nThreads, 0ULL);

        clock_t t0 = clock();
        int result = parallelAStar_hybrid(puzzle, nThreads, d_split);
        clock_t t1 = clock();

        double secs = double(t1 - t0) / CLOCKS_PER_SEC;

        unsigned long long totalExpanded = 0;
        for (auto v : nodesExpandedPerThread)
            totalExpanded += v;

        cout << setw(6) << caseNum
             << setw(20) << puzzle
             << setw(10) << result
             << setw(18) << totalExpanded
             << setw(25);

        // Mostrar nodos expandidos por hilo
        for (size_t i = 0; i < nodesExpandedPerThread.size(); ++i)
            cout << "[" << i << ":" << nodesExpandedPerThread[i] << "] ";

        cout << setw(15) << fixed << setprecision(6) << secs
             << setw(12) << nThreads
             << endl;

        caseNum++;
    }

    fin.close();
    return 0;
}

