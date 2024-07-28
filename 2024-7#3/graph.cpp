#include "graph.hpp"

int v2id(int r, int c, int cols) { return r * cols + c; }

pii id2v(int id, int cols) { return {id / cols, id % cols}; }

// Graph

bool Graph::reachable(int r, int c) { return grid[r][c] == 'o'; }

bool Graph::input(string filename) {
    {
        // input map
        ifstream fin(filename);
        if (!fin) return false;
        string line;
        while (getline(fin, line)) { grid.emplace_back(vector<char>(line.begin(), line.end())); }

        fin.close();
    }
    // transfer grid to graph = (V, E)
    rows = grid.size();
    cols = grid[0].size();
    nodes = rows * cols;
    graph.resize(nodes);
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (!reachable(r, c)) continue;
            val u = v2id(r, c, cols);
            for (int i = 0; i < 4; i++) {
                val tr = r + dr[i];
                val tc = c + dc[i];
                if (tr < 0 || tr >= rows || tc < 0 || tc >= cols) continue;
                if (!reachable(tr, tc)) continue;
                val v = v2id(tr, tc, cols);
                graph[u].push_back(v);
                val id = id2v(v, cols);
            }
        }
    }
    return true;
}

void Graph::solveShortestPath() {
    fun bfs = [=](int start) {
        vector<int> dis(graph.size(), INT_SOFT_MAX);
        vector<int> vis(graph.size(), 0);
        dis[start] = 0;
        queue<int> q;
        q.push(start);
        vis[start] = 1;
        while (!q.empty()) {
            val u = q.front();
            q.pop();
            for (val v : graph[u]) {
                if (vis[v]) continue;
                dis[v] = dis[u] + 1;
                vis[v] = 1;
                q.push(v);
            }
        }
        return dis;
    };
    //
    dist.resize(graph.size());
    for (var u = 0; u < graph.size(); u++) { dist[u] = bfs(u); }
}

int Graph::shortestPath(int fromR, int fromC, int toR, int toC) const {
    return dist[v2id(fromR, fromC, cols)][v2id(toR, toC, cols)];
}

vector<pii> Graph::traceSimplePath(int fromR, int fromC, int toR, int toC) const {
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
            vis[u] = 1;
            for (val v : graph[u]) {
                if (vis[v]) continue;
                pre[v] = u;
                if (v == goalId) return;
                val vv = id2v(v, cols);
                q.push({dis + 1 + shortestPath(vv[0], vv[1], toR, toC), v});
            }
        }
    }();
    // trace
    vector<pii> ret;
    for (var u = goalId; u != -1; u = pre[u]) ret.push_back(id2v(u, cols));
    reverse(ret.begin(), ret.end());
    return ret;
}
