#pragma once

#include <algorithm>
#include <array>
#include <ctime>
#include <fstream>
#include <iostream>
#include <queue>
#include <random>
#include <set>
#include <string>
#include <vector>

using std::array, std::queue, std::string, std::vector, std::pair, std::priority_queue, std::set;
using std::make_pair;

using std::cout, std::endl, std::cerr, std::ifstream, std::ofstream;

using std::getline, std::max, std::min, std::move;

using std::mt19937, std::random_device, std::uniform_int_distribution, std::uniform_real_distribution;

using std::reverse;

template <typename T> using matrix = vector<vector<T>>;

template <typename T> using ref = const T &;

using pii = array<int, 2>;

// just for fun
#define var auto
#define val const auto
#define fun auto
#define loop while (1)

#define isStr(X) (sizeof(#X) > 2 && (#X)[0] == '"' && #X[sizeof(#X) - 2] == '"')

#define DEBUG(X)                                                                                                       \
    do {                                                                                                               \
        if (isStr(X))                                                                                                  \
            std::cerr << __FILE__ << "#" << __LINE__ << ": " << X << std::endl;                                        \
        else                                                                                                           \
            std::cerr << __FILE__ << "#" << __LINE__ << ": " << #X << " -> " << (X) << std::endl;                      \
    } while (0)

// for agv schedule
const string map_file = "map.txt";
const string order_file = "order.txt";

const string sa4lowerbound_file = "sa4lowerbound.txt";
const string sa4lowerbound_path_file = "sa4lowerbound_path.txt";
const string greedy4simulate_path_file = "greedy4simulate_path.txt";

const int MAX_AGV = 4;
const int MAX_ORDER = 100;
const int MAX_AGV_TASK = 5;

const vector<pii> agvRestArea = {
    {1, 1},
    {2, 1},
    {3, 1},
    {4, 1},
};
const pii sendArea = {12, 2};
