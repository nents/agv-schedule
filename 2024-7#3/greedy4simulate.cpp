#include "greedy4simulate.hpp"

// structs
// GraphG
vector<pii> GraphG::traceBlockedPath(ref<set<pii>> blocks, int fromR, int fromC, int toR, int toC) const {
    if (grid[toR][toC] != 'o') return {};
    fun isBlock = [&](pii v) { return blocks.find(v) != blocks.end(); };
    if (isBlock(pii{toR, toC})) return {};
    priority_queue<pii, vector<pii>, std::greater<pii>> q;
    q.push({0, v2id(fromR, fromC, cols)});
    vector<int> vis(nodes, 0);
    vector<int> pre(nodes, -1);
    val goalId = v2id(toR, toC, cols);
    // A*
    [&]() {
        while (!q.empty()) {
            auto [dis, u] = q.top();
            q.pop();
            if (vis[u]) continue;
            vis[u] = 1;
            for (val v : graph[u]) {
                if (vis[v]) continue;
                val vv = id2v(v, cols);
                if (isBlock(vv)) continue;
                pre[v] = u;
                if (v == goalId) return;
                q.push({dis + 1 + shortestPath(vv[0], vv[1], toR, toC), v});
            }
        }
    }();
    // trace
    vector<pii> ret;
    for (var u = goalId; u != -1; u = pre[u]) { ret.push_back(id2v(u, cols)); }
    reverse(ret.begin(), ret.end());
    return ret;
}
pii GraphG::argAdjMin(int fromR, int fromC, int toR, int toC) const {
    int mn = INT_SOFT_MAX, mnIdx = 0;
    for (var i = 0; i < 4; i++) {
        int tr = toR + dr[i], tc = toC + dc[i];
        if (tr < 0 || tr >= rows || tc < 0 || tc >= cols) continue;
        int distance = shortestPath(fromR, fromC, tr, tc);
        if (distance < mn) {
            mn = distance;
            mnIdx = i;
        }
    }
    return {mn, mnIdx};
}
// Agv
void Agv::setPosition(int r, int c) { position = {r, c}; }
//

void greedy4simulate() {
    DEBUG("begin greedy4simulate");

    // constants
    const int SIM_CLOCK_MAX = 1000;

    // variables
    GraphG G;
    OrderG O;

    matrix<pii> paths(MAX_AGV);
    vector<Agv> agvs;

    int sim_clock = 0;

    // init
    G.input(map_file);
    G.solveShortestPath();
    O.input(order_file);

    // simulate
    agvs.resize(MAX_AGV);
    for (var index = 0; index < MAX_AGV; index++) {
        var &agv = agvs[index];
        agv.restPosition = agvRestArea[index];
        agv.position = agv.restPosition;
        agv.id = index;
        agv.target = agv.restPosition;
    }

    loop {
        // limit clocks
        if ((++sim_clock) > SIM_CLOCK_MAX) break;
        // report agvs
        for (var idx = 0; idx < MAX_AGV; idx++) {
            val &agv = agvs[idx];
            paths[idx].push_back(agv.position);
        }
        // update blocks
        set<pii> blocks;
        for (val &agv : agvs) { blocks.insert(agv.position); }
        // try to assign a task MAX_AGV times
        // fetch all free agv index
        vector<int> freeAgvIdx;
        for (var idx = 0; idx < MAX_AGV; idx++) {
            val agv = agvs[idx];
            if (agv.target == agv.restPosition || agv.position == agv.restPosition ||
                (agv.target == sendArea && agv.storage != MAX_AGV_TASK))
                freeAgvIdx.push_back(idx);
        }
        // try to assign work
        while (O.nextAssignIndex != MAX_ORDER) {
            var succ = false;
            pii task = O.order[O.nextAssignIndex];
            for (var idx : freeAgvIdx) {
                var &agv = agvs[idx];
                // try to assign a work
                auto [dis, dt] = G.argAdjMin(agv.position[0], agv.position[1], task[0], task[1]);
                if (agv.target == sendArea) {
                    val sDis = G.shortestPath(task[0] + dr[dt], task[1] + dc[dt], sendArea[0], sendArea[1]);
                    val newDis = dis + sDis;
                    val oldDis = G.shortestPath(agv.position[0], agv.position[1], sendArea[0], sendArea[1]) + 2 * sDis;
                    if (newDis < oldDis) {
                        agv.target = {task[0] + dr[dt], task[1] + dc[dt]};
                        freeAgvIdx.erase(std::find(freeAgvIdx.begin(), freeAgvIdx.end(), agv.id));
                        succ = true;
                        O.nextAssignIndex++;
                    }
                    if (succ) break;
                }
                if (agv.target == agv.restPosition) {
                    auto [dis, dt] = G.argAdjMin(agv.position[0], agv.position[1], task[0], task[1]);
                    agv.target = {task[0] + dr[dt], task[1] + dc[dt]};
                    freeAgvIdx.erase(std::find(freeAgvIdx.begin(), freeAgvIdx.end(), agv.id));
                    succ = true;
                    O.nextAssignIndex++;
                    if (succ) break;
                }
            }
            if (!succ || freeAgvIdx.empty()) break;
        }
        // move agvs
        for (var idx = 0; idx < MAX_AGV; idx++) {
            var &agv = agvs[idx];
            if (agv.position == agv.target || agv.target == pii{0, 0}) continue;
            val trace = G.traceBlockedPath(blocks, agv.position[0], agv.position[1], agv.target[0], agv.target[1]);
            val nextPos = (trace.empty() || trace.size() == 1 ? agv.position : trace[1]);
            agv.position = nextPos;
            blocks.insert(nextPos);
        }
        // check arrival of targe
        for (var &agv : agvs) {
            if (agv.position != agv.target) continue;
            if (agv.target == agv.restPosition) continue;
            if (agv.target == sendArea) {
                agv.target = agv.restPosition;
                agv.storage = 0;
            } else {
                if (agv.target == agv.position) agv.storage++;
                agv.target = sendArea;
            }
        }

        // break
        if (O.nextAssignIndex == MAX_ORDER) {
            var doneAgvs = 0;
            for (val &agv : agvs) {
                if (agv.position == agv.target) doneAgvs++;
            }
            if (doneAgvs == MAX_AGV) break;
        }
    }
    DEBUG("final:");
    DEBUG(sim_clock);

    // ouput paths
    ofstream fout(greedy4simulate_path_file);
    for (var idx = 0; idx < MAX_AGV; idx++) {
        val &path = paths[idx];
        for (val p : path) { fout << p[0] << " " << p[1] << " "; }
        fout << endl;
    }
    fout.close();

    DEBUG("end greedy4simulate");
}