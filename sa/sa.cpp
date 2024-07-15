#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;

#define DEBUG(X) \
    do { cerr << __FILE__ << "#" << __LINE__ << " " << #X << " -> " << X << endl; } while (0)

template <typename T>
using matrix = vector<vector<T>>;

typedef pair<int, int> pii;

const string mapFile = "map.txt";
const string orderFile = "order.txt";
const int INT_SOFT_MAX = 1e9 + 7;
const int dc[] = {0, 1, 0, -1};
const int dr[] = {-1, 0, 1, 0};

const double INIT_T = 1200;
const double DELTA_T = 0.997;
const int MAX_ITER = 3;
const double eps = 1e-7;

const int MAX_AGV = 4;
const int MAX_AGV_TASK = 4;

mt19937 gen;

int toNode(const matrix<char> &map, int r, int c) {
    return r * map[0].size() + c;
}

double dp(const matrix<char> &map, const matrix<int> &floyd, const vector<pii> &order, const vector<int> &schedule) {
    auto to_node = [=](int r, int c) { return toNode(map, r, c); };
    auto dis = [=](int fromR, int fromC, int toR, int toC) { return floyd[to_node(fromR, fromC)][to_node(toR, toC)]; };
    auto cost = [=](int from, int to) {
        int ret = 0;
        int lastR = 1, lastC = 1;
        for (int i = from; i <= to; i++) {
            int r = order[i].first, c = order[i].second;
            int mn = INT_SOFT_MAX, mnIdx = -1;
            for (int j = 0; j < 4; j++) {
                int nr = r + dr[j], nc = c + dc[j];
                if (nr < 0 || nr >= map.size() || nc < 0 || nc >= map[0].size()) continue;
                int dist = dis(lastR, lastC, nr, nc);
                if (dist < mn) {
                    mn = dist;
                    mnIdx = j;
                }
            }
            ret += mn;
            lastR = r + dr[mnIdx];
            lastC = c + dc[mnIdx];
        }
        ret += dis(lastR, lastC, 1, 1);
        return ret;
    };

    // evaluate the arrangement
    double ret = 0;

    for (int idx = 0; idx < MAX_AGV; idx++) {
        vector<int> task;
        for (int i = 0; i < schedule.size(); i++) {
            int v = schedule[i];
            if (v == idx) { task.emplace_back(i); }
        }
        if (task.empty()) continue;
        vector<int> f(task.size(), INT_SOFT_MAX);
        // dp
        double time = 0;
        f[0] = cost(0, 0);
        for (int i = 1; i < task.size(); i++) {
            for (int j = i; j >= 0 && i - j < MAX_AGV_TASK; j--) {
                int c = cost(j, i);
                if (f[i] > f[j] + c) {
                    time += c;
                    f[i] = f[j] + (time * 0.7 + c * 0.3);
                }
            }
        }
        ret += f[task.size() - 1];
    }

    return ret;
}

int main() {

    // variables
    matrix<char> inputMap;  // intputMap[row][col]
    vector<pii> inputOrder; // [(row, col)]

    // set seed
    gen.seed(random_device()());

    ifstream fin;
    string line;

    // input map
    fin.open(mapFile);
    while (getline(fin, line)) {
        inputMap.emplace_back(vector<char>(line.begin(), line.end()));
    }
    fin.close();

    // input order
    fin.open(orderFile);
    int col, row;
    while (fin >> col >> row) {
        inputOrder.emplace_back(col, row);
    }
    fin.close();

    DEBUG("input done");

    // trans grid matrix to adj matrix
    int rows = inputMap.size();
    int cols = inputMap[0].size();
    int nodes = rows * cols;
    matrix<int> floydMap(nodes, vector<int>(nodes, INT_SOFT_MAX));

    auto to_node = [=](int r, int c) { return toNode(inputMap, r, c); };

    for (int i = 0; i < nodes; i++) { floydMap[i][i] = 0; }
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (inputMap[r][c] != 'o') continue;
            int u = to_node(r, c);
            for (int i = 0; i < 4; i++) {
                int tr = r + dr[i];
                int tc = c + dc[i];
                if (tr < 0 || tr >= rows || tc < 0 || tc >= cols) continue;
                if (inputMap[tr][tc] != 'o') continue;
                int v = to_node(tr, tc);
                floydMap[u][v] = 1;
            }
        }
    }
    // floyd
    for (int k = 0; k < nodes; k++) {
        for (int i = 0; i < nodes; i++) {
            for (int j = 0; j < nodes; j++) {
                floydMap[i][j] = min(floydMap[i][j], floydMap[i][k] + floydMap[k][j]);
            }
        }
    }

    DEBUG("floyd done");

    // sa
    auto evaluate = [=](const vector<int> &schedule) { return dp(inputMap, floydMap, inputOrder, schedule); };
    auto feq = [](double x, double y) { return abs(x - y) < eps; };

    uniform_int_distribution<int> randAgv(0, MAX_AGV - 1);
    uniform_int_distribution<int> randOrder(0, inputOrder.size() - 1);
    uniform_real_distribution<double> dist(0, 1);
    double gBestAns = INT_SOFT_MAX;

    vector<int> zeros(inputOrder.size(), 0);
    DEBUG(evaluate(zeros));

    for (int times = 0; times < MAX_ITER; times++) {
        double t = INIT_T;
        vector<int> schedule(inputOrder.size(), 0);
        for (int &v : schedule) { v = randAgv(gen); }
        double bestAns = evaluate(schedule);

        while (t > eps) {
            int pos = randOrder(gen);
            int redo = schedule[pos];
            schedule[pos] = randAgv(gen);
            int nowAns = evaluate(schedule);
            int delta = nowAns - bestAns;
            if (delta < 0 || exp(-delta / t) > dist(gen)) {
                bestAns = nowAns;
            } else {
                schedule[pos] = redo;
            }
            t *= DELTA_T;

            // output t - ans
            if (times == 0 && (dist(gen) < 0.001 && t > 0.001 || feq(t, 1196.4))) { // 0.1%
                DEBUG(t);
                DEBUG(bestAns);
            }
        }
        gBestAns = min(gBestAns, bestAns);

        DEBUG("sa done");
        DEBUG(times);
        DEBUG(bestAns);
    }

    // out

    cout << "best ans: " << gBestAns << endl;

    return 0;
}