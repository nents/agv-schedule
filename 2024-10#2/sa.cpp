// sa

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

#define int long long

const string input = "large_1000_gradian";
const string input_file = input + ".in";
const string output_file = input + "_sa.out";

const int _ms = 1;
const int _s = 1000 * _ms;
const int _m = 60 * _s;
const int _h = 60 * _m;
const int evaluateTime = 180 * _s;

const double eps = 1e-10;
const double t0 = 1e11, delta_t = 0.999997;

int n, ans;
vector<int> w, p;

random_device rd;
mt19937 gen(rd());

void disturb(vector<int> &p) {
    shuffle(p.begin() + 1, p.end() - 1, gen);
    p[0] = p[n];
    p[n + 1] = p[1];
}

int evaluate(const vector<int> &p, const vector<int> &w) {
    int ret = 0;
    for (int i = 1; i <= n; i++) { ret += p[i] * w[p[i - 1]] * w[p[i]] * w[p[i + 1]]; }
    return ret;
}

void delta_value(int x, int y) {
    int A = p[x > 2 ? x - 2 : x + n - 2];
    int B = p[x > 1 ? x - 1 : n];
    int &C = p[x];
    int D = p[x < n ? x + 1 : 1];
    int E = p[x < n - 1 ? x + 2 : x - n + 2];

    ans -= (w[A] * w[B] * B + w[B] * C * w[D] + D * w[D] * w[E]) * w[C];
    C = y;
    ans += (w[A] * w[B] * B + w[B] * C * w[D] + D * w[D] * w[E]) * w[C];
}

signed main() {

    freopen(input_file.c_str(), "r", stdin);
    freopen(output_file.c_str(), "w", stdout);

    auto start = chrono::high_resolution_clock::now();

    // input
    cin >> n;
    w.resize(n + 2);
    p.resize(n + 2);
    for (int i = 1; i <= n; i++) { cin >> w[i]; }
    for (int i = 0; i <= n + 1; i++) { p[i] = i; }

    // solve

    disturb(p);
    ans = evaluate(p, w);
    double t = t0;

    uniform_int_distribution<int> uniform_int(1, n);
    uniform_real_distribution<double> uniform(0, 1);
    auto random_pos = [&]() { return uniform_int(gen); };
    auto random_01 = [&]() { return uniform(gen); };

    auto duration = [&]() {
        return chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count();
    };
    while (duration() < evaluateTime) {
        if (t < eps) { t = t0; }

        int x = random_pos(), y = random_pos();
        int last = ans;
        while (x == y) { y = random_pos(); }

        int px = p[x], py = p[y];
        // delta_value(x, py);
        // delta_value(y, px);
        swap(p[x], p[y]);
        ans = evaluate(p, w);

        int delta_ans = ans - last;
        if (delta_ans > 0 || exp((delta_ans - ans) / t) > random_01()) {
            ;
        } else {
            ans = last;
            swap(p[x], p[y]);
        }

        t *= delta_t;
    }

    cout << "ans: " << ans << endl;
    cout << endl
         << "duration: "
         << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count() << " ms"
         << endl;
    for (int i = 1; i <= n; i++) { cout << p[i] << " "; }



    return 0;
}