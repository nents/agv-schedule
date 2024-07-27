#pragma once

#include "top.h"

const int dr[] = {-1, 0, 1, 0};
const int dc[] = {0, 1, 0, -1};
const int INT_SOFT_MAX = 1e9 + 7;

struct Graph {
    matrix<char> grid;
    matrix<int> graph;
    matrix<int> dist; // shortest distance, using id
    int cols = 0, rows = 0;
    int nodes = 0;
    //
    Graph() = default;
    //
    int shortestPath(int fromR, int fromC, int toR, int toC) const;
    bool input(string filename);
    bool reachable(int r, int c);
    void solveShortestPath();
    vector<pii> traceSimplePath(int fromR, int fromC, int toR, int toC) const;
};

int v2id(int r, int c, int cols);
pii id2v(int id, int cols);