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

const string input_file = "3.in";

const int max_iter = 233333;
const int kMax = 5;

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

void neighborhoodSwap(vector<int> &s) {
    int n = s.size() - 2;
    auto randomPos = uniform_int_distribution<int>(1, n);
    int i = randomPos(mt);
    int j = randomPos(mt);
    while (i == j) { j = randomPos(mt); }
    swap(s[i], s[j]);
    s[0] = s[n];
    s[n + 1] = s[1];
}

void neighborhoodKSwap(vector<int> &s, int k) {
    int n = s.size() - 2;
    auto randomPos = uniform_int_distribution<int>(1, n);
    for (int _ = 0; _ < k; _++) {
        int i = randomPos(mt);
        int j = randomPos(mt);
        while (i == j) { j = randomPos(mt); }
        swap(s[i], s[j]);
    }
    s[0] = s[n];
    s[n + 1] = s[1];
}

void neighborhoodInsert(vector<int> &s) {
    int n = s.size() - 2;
    auto randomPos = uniform_int_distribution<int>(1, n);
    int i = randomPos(mt);
    int j = randomPos(mt);
    while (i == j) { j = randomPos(mt); }
    int tmp = s[i];
    s.erase(s.begin() + i);
    s.insert(s.begin() + j, tmp);
    s[0] = s[n];
    s[n + 1] = s[1];
}

void neighborhoodReverse(vector<int> &s) {
    int n = s.size() - 2;
    auto randomPos = uniform_int_distribution<int>(1, n);
    int i = randomPos(mt);
    int j = randomPos(mt);
    while (i == j) { j = randomPos(mt); }
    if (i > j) { swap(i, j); }
    reverse(s.begin() + i, s.begin() + j + 1);
    s[0] = s[n];
    s[n + 1] = s[1];
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

vector<int> adaptiveNeighborhoodSearch(const vector<int> &w, int max_iter) {
    // record P
    vector<function<void(vector<int> &)>> neighborhoods = {neighborhoodSwap, neighborhoodInsert, neighborhoodReverse,
                                                           neighborhoodShuffle};
    vector<int> success(neighborhoods.size(), 1);
    auto dist01 = uniform_real_distribution<double>(0.0, 1.0);
    auto random01 = [&]() { return dist01(mt); };

    // ANS
    int n = w.size() - 2;
    vector<int> bestSolution = generateSolution(n);
    int bestValue = evaluate(w, bestSolution);
    vector<int> currentSolution = bestSolution;
    int currentValue = bestValue;
    for (int iter = 0; iter < max_iter; iter++) {
        // choose neighborhood
        vector<double> p(neighborhoods.size(), 0.0);
        double total = accumulate(success.begin(), success.end(), 0.0);
        for (int i = 0; i < neighborhoods.size(); i++) { p[i] = success[i] / total; }

        double r = random01();
        double cumulativeP = 0.0;
        int selectedNeighborhood = 0;
        while (selectedNeighborhood < neighborhoods.size()) {
            cumulativeP += p[selectedNeighborhood];
            if (r <= cumulativeP) break;
            selectedNeighborhood++;
        }

        // evaluate
        neighborhoods[selectedNeighborhood](currentSolution);
        int currentValue = evaluate(w, currentSolution);

        if (currentValue > bestValue) {
            bestSolution = currentSolution;
            bestValue = currentValue;
            success[selectedNeighborhood]++;
        }
    }

    // cout << "succ times: " << endl;
    // cout << "swap: " << success[0] << endl;
    // cout << "insert: " << success[1] << endl;
    // cout << "reverse: " << success[2] << endl;
    // cout << "shuffle: " << success[3] << endl;

    return bestSolution;
}

vector<int> variableNeighborhoodSearch(const vector<int> &w, int max_iter) {
    int n = w.size() - 2;
    vector<int> bestSolution = generateSolution(n);
    int bestValue = evaluate(w, bestSolution);


    auto randomKSwap = uniform_int_distribution<int>(1, sqrt(n));
    for (int iter = 0; iter < max_iter; iter++) {
        int k = 1;

        while (k <= kMax) {
            vector<int> currentSolution = bestSolution;

            auto judgeSolution = [&]() {
                int currentValue = evaluate(w, currentSolution);
                if (currentValue > bestValue) {
                    bestSolution = currentSolution;
                    bestValue = currentValue;
                    return true;
                } else {
                    return false;
                }
            };

            // force to explore nearest neighborhood
            // for (int _ = 0; _ < 2; _++) {
            //     judgeSolution();
            // }

            // explore neighborhood
            if (k == 1) { neighborhoodSwap(currentSolution); }
            if (k == 2) { neighborhoodKSwap(currentSolution, randomKSwap(mt)); }
            if (k == 3) { neighborhoodInsert(currentSolution); }
            if (k == 4) { neighborhoodReverse(currentSolution); }
            if (k == 5) { neighborhoodShuffle(currentSolution); }

            k = judgeSolution() ? 1 : k + 1;
        }
    }

    return bestSolution;
}


signed main() {

    freopen(input_file.c_str(), "r", stdin);

    // input
    int n;
    cin >> n;
    vector<int> w(n + 2);
    for (int i = 1; i <= n; i++) { cin >> w[i]; }

    // solve
    auto start = chrono::high_resolution_clock::now();

    vector<int> solution = variableNeighborhoodSearch(w, max_iter);

    for (int i = 1; i <= n; i++) { cout << solution[i] << " "; }

    cout << endl << "ans: " << evaluate(w, solution) << endl;
    cout << "duration: "
         << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count() << "ms"
         << endl;

    return 0;
}