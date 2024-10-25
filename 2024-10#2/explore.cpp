#include "evaluate.hpp"

const string input = "large_1000_random";
const string input_file = input + ".in";
const string output_file = input + "_best.out";

const int _ms = 1;
const int _s = 1000 * _ms;
const int _m = 60 * _s;
const int _h = 60 * _m;
const int evaluateTime = 8 * _h;

const double eps = 1e-13;
const double t0 = 1e17, delta_t = 0.999997;

random_device rd;
mt19937 mt(rd());

int gSize = 0, gValue = 0;
vector<int> gWeight;

void solveGlobalAnswer(int value, const vector<int> &s) {
    if (value > gValue) {
        gValue = value;
        // output
        ofstream fout(output_file);
        fout << value << endl;
        for (int i = 1; i <= gSize; i++) { fout << s[i] << " "; }
        fout.close();
    }
}

vector<int> generatePermutation(int size) {
    vector<int> permutation(size + 2);
    iota(permutation.begin() + 1, permutation.end() - 1, 1);
    shuffle(permutation.begin() + 1, permutation.end() - 1, mt);
    permutation[0] = permutation[size];
    permutation[size + 1] = permutation[1];
    return permutation;
}

int evaluate(const vector<int> &w, const vector<int> &p) {
    int n = p.size() - 2;
    int ret = 0;
    for (int i = 1; i <= n; i++) { ret += p[i] * w[p[i - 1]] * w[p[i]] * w[p[i + 1]]; }
    return ret;
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

Task solveShuffle() {
    loop {
        vector<int> permutation = generatePermutation(gSize);
        int value = evaluate(gWeight, permutation);
        solveGlobalAnswer(value, permutation);
        co_await suspend_always();
    }
}

Task simulatedAnnealing() {
    vector<int> solution = generatePermutation(gSize);
    int best = evaluate(gWeight, solution);
    double t = t0;

    uniform_int_distribution<int> uniform_int(1, gSize);
    uniform_real_distribution<double> uniform(0, 1);
    auto random_pos = [&]() { return uniform_int(mt); };
    auto random_01 = [&]() { return uniform(mt); };

    auto solutionSwap = [&](int i, int j) {
        swap(solution[i], solution[j]);
        solution[0] = solution[gSize];
        solution[gSize + 1] = solution[1];
    };
    loop {
        if (t < eps) {
            t = t0;
            solution = generatePermutation(gSize);
        }
        int x = random_pos(), y = random_pos();
        while (x == y) { y = random_pos(); }
        solutionSwap(x, y);

        int current = evaluate(gWeight, solution);
        int delta = current - best;
        if (delta > 0 || exp(delta / t) > random_01()) {
            best = current;
            solveGlobalAnswer(best, solution);
        } else {
            solutionSwap(x, y);
        }
        t *= delta_t;

        co_await suspend_always();
    }
}

Task variableNeighborhoodSearch() {
    vector<int> bestSolution = generatePermutation(gSize);
    int best = evaluate(gWeight, bestSolution);
    const int kMax = 4;

    loop {
        int k = 1;
        while (k <= kMax) {
            vector<int> solution = bestSolution;
            if (k == 1) {
                neighborhoodSwap(solution);
            } else if (k == 2) {
                neighborhoodInsert(solution);
            } else if (k == 3) {
                neighborhoodReverse(solution);
            } else if (k == 4) {
                neighborhoodShuffle(solution);
            }

            int value = evaluate(gWeight, solution);
            if (value > best) {
                best = value;
                bestSolution = solution;
                k = 1;
                solveGlobalAnswer(best, bestSolution);
            } else {
                k++;
            }
        }

        co_await suspend_always();
    }
}

signed main() {

    // intput
    ifstream fin(input_file);
    fin >> gSize;
    gWeight.resize(gSize + 2);
    for (int i = 1; i <= gSize; i++) { fin >> gWeight[i]; }
    fin.close();
    // evaluate

    Task shuffle = solveShuffle();
    Task SA = simulatedAnnealing();
    Task VNS = variableNeighborhoodSearch();

    int iter = 0;
    auto start = chrono::high_resolution_clock::now();
    auto duration = [&]() {
        auto end = chrono::high_resolution_clock::now();
        return chrono::duration_cast<chrono::milliseconds>(end - start).count();
    };

    loop {
        shuffle.resume();
        SA.resume();
        VNS.resume();
        printf("iter: %lld --> best: %lld\n", ++iter, gValue);
        if (duration() > evaluateTime) { break; }
    }


    return 0;
}