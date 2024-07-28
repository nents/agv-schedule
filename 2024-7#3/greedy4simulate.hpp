#pragma once

#include "graph.hpp"
#include "order.hpp"
#include "top.hpp"

struct GraphG : Graph {
    vector<pii> traceBlockedPath(ref<set<pii>> blocks, int fromR, int fromC, int toR, int toC) const;
    pii argAdjMin(int fromR, int fromC, int toR, int toC) const; // return {dis, arg}
};

struct OrderG : Order {
    int nextAssignIndex = 0;
};

struct Agv {
    pii position;
    pii target;
    pii restPosition;
    int id;
    int storage;
    //
    void setPosition(int r, int c);
};

void greedy4simulate();