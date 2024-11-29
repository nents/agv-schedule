#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <queue>
#include <random>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>



// defines

#define DEBUG(X)                                                                                                       \
    do { cerr << "[#" << __LINE__ << "] " << #X << " -> " << (X) << endl; } while (false)

#define int int64_t

// usings

using std::apply, std::make_pair, std::pair, std::set, std::string, std::string_view, std::tie, std::unordered_map,
    std::vector, std::priority_queue, std::queue, std::unordered_set;

using std::cerr, std::cout, std::endl, std::cin;

using std::getline;

using std::ifstream, std::ofstream;

using std::ranges::reverse, std::hash, std::ranges::view, std::views::iota;

using std::max, std::min, std::greater;

using std::uniform_int_distribution, std::uniform_real_distribution, std::shuffle;

// templates

using pii = pair<int, int>;

template <typename T> using matrix = vector<vector<T>>;

template <typename T> void vectorPrint(const vector<T> &v) {
    for (auto elem : v) { cerr << elem; }
    cout << endl;
}

template <typename T> void matrixPrint(const matrix<T> &m) {
    for (auto v : m) { vectorPrint(v); }
}

template <typename T> void matrixResize(matrix<T> &m, int rows, int cols, const T defaultValue) {
    m.resize(rows);
    for (auto &row : m) { row.resize(cols, defaultValue); }
}

namespace std {
template <> struct hash<pii> {
    size_t operator()(const pii &p) const { return hash<int>()(p.first) ^ (hash<int>()(p.second) << 1); }
};
} // namespace std