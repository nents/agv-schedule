#include <cmath>
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

const int INT_SOFT_MAX = 1e9 + 7;
const double eps = 1e-8;

struct Point {
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}
};


bool feq(double x, double y) { return fabs(x - y) < eps; }

double sqr(double x) { return x * x; }

Point calc(Point i, Point j) {
    double a = (j.y * i.x - i.y * j.x) / (sqr(j.x) * i.x - sqr(i.x) * j.x);
    double b = (i.y - a * sqr(i.x)) / i.x;
    return Point(a, b);
}

int lowestZero(int x) {
    int ret = 0;
    if (x == 0) return 0;
    for (; x & (1 << ret); ret++);
    return ret;
}

int main() {

    // variables
    vector<Point> pigs;

    // input
    ifstream fin("pigs.txt");
    double x, y;
    while (fin >> x >> y) { pigs.push_back(Point(x, y)); }
    fin.close();

    DEBUG(pigs.size());

    // solve
    vector<int> f(1 << pigs.size(), INT_SOFT_MAX);
    matrix<int> g(pigs.size() + 1, vector<int>(pigs.size() + 1, 0));
    f[0] = 0;

    for (int i = 0; i < pigs.size(); i++) {
        for (int j = 0; j < pigs.size(); j++) {
            if (feq(pigs[i].x, pigs[j].x)) continue;
            Point line = calc(pigs[i], pigs[j]); // {x, y} = {a, b} => ax^2 + bx
            if (line.x > -eps) continue;
            for (int k = 0; k < pigs.size(); k++) {
                if (feq(line.x * sqr(pigs[k].x) + line.y * pigs[k].x, pigs[k].y)) { g[i][j] |= (1 << k); }
            }
        }
    }
    for (int S = 0; S < (1 << pigs.size()); S++) {
        int i = lowestZero(S);
        f[S | (1 << i)] = min(f[S | (1 << i)], f[S] + 1);
        for (int j = 0; j < pigs.size(); j++) { f[S | g[i][j]] = min(f[S | g[i][j]], f[S] + 1); }
    }

    // output
    cout << f[(1 << pigs.size()) - 1];

    return 0;
}