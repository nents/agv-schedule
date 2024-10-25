// vns

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

using namespace std;

#define int long long

const string input = "large_1000_gradian";
const string input_file = input + ".in";
const string output_file = input + "_shuffle.out";


const int _ms = 1;
const int _s = 1000 * _ms;
const int _m = 60 * _s;
const int _h = 60 * _m;
const int evaluateTime = 180 * _s;

random_device rd;
mt19937 mt(rd());

vector<int> generateSolution(int size) {
    vector<int> solution(size + 2);
    iota(solution.begin() + 1, solution.end() - 1, 1);
    shuffle(solution.begin() + 1, solution.end() - 1, mt);
    solution[0] = solution[size];
    solution[size + 1] = solution[1];
    return solution;
}

void neighborhoodShuffle(vector<int> &s) {
    int n = s.size() - 2;
    auto randomPos = uniform_int_distribution<int>(1, n);
    int i = randomPos(mt);
    int j = randomPos(mt);
    while (i == j) { j = randomPos(mt); }
    if (i > j) { swap(i, j); }
    s[0] = s[n];
    s[n + 1] = s[1];
}


int evaluate(const vector<int> &w, const vector<int> &p) {
    int n = p.size() - 2;
    int ret = 0;
    for (int i = 1; i <= n; i++) { ret += p[i] * w[p[i - 1]] * w[p[i]] * w[p[i + 1]]; }
    return ret;
}

vector<int> solveShuffle(const vector<int> &w) {
    int n = w.size() - 2;
    vector<int> current = generateSolution(n);
    vector<int> best = current;
    int bestValue = evaluate(w, best);

    auto start = chrono::high_resolution_clock::now();
    auto duration = [&]() {
        return chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count();
    };

    while (duration() < evaluateTime) {
        neighborhoodShuffle(current);
        int currentValue = evaluate(w, current);
        if (currentValue < bestValue) {
            best = current;
            bestValue = currentValue;
        }
    }

    return best;
}


signed main() {

    freopen(input_file.c_str(), "r", stdin);
    freopen(output_file.c_str(), "w", stdout);

    // input
    int n;
    cin >> n;
    vector<int> w(n + 2);
    for (int i = 1; i <= n; i++) { cin >> w[i]; }

    // solve
    auto start = chrono::high_resolution_clock::now();

    vector<int> solution = solveShuffle(w);

    cout << "ans: " << evaluate(w, solution) << endl;
    cout << "duration: "
         << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count() << "ms"
         << endl;
    for (int i = 1; i <= n; i++) { cout << solution[i] << " "; }



    return 0;
}