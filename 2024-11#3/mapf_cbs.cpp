#include "mapf.hpp"

// constants

const int INF = 1e18;
const int dr[] = {0, 1, 0, -1};
const int dc[] = {1, 0, -1, 0};

const unordered_map<char, int> mapMap = {{'#', 0}, {'$', 1}, {'o', 2}, {'P', 3}};

// globals

std::random_device rd;
std::mt19937 mt(rd());

// structs

struct Clock {
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::microseconds totalDuration = std::chrono::microseconds::zero();
    bool running = false;

    Clock() = default;

    void start() {
        if (!running) {
            startTime = std::chrono::steady_clock::now();
            running = true;
        }
    }

    void stop() {
        if (running) {
            totalDuration +=
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - startTime);
            running = false;
        }
    }

    void reset() {
        totalDuration = std::chrono::microseconds::zero();
        running = false;
    }

    int duration() const {
        if (running) {
            return totalDuration.count() +
                   std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - startTime)
                       .count();
        } else {
            return totalDuration.count();
        }
    }

    double durationMus() const { return (double)duration(); }

    double durationMs() const { return durationMus() / 1000.0; }

    double durationSec() const { return durationMs() / 1000.0; }

    double durationMin() const { return durationSec() / 60.0; }
};

struct Map {
    string dir = "";
    int cols = 0;
    int rows = 0;
    matrix<int> map;

    Map(string_view dir) : dir(dir) {}

    int toIndex(int r, int c) const { return r * cols + c; }
    pii toCoord(int index) const { return {index / cols, index % cols}; }

    void read() {
        ifstream fin(dir);
        if (!fin.is_open()) {
            DEBUG("Map file not found");
            exit(1);
        }

        string line;
        getline(fin, line);
        cols = line.size();
        rows = 0;
        do {
            vector<int> row(cols, 0);
            for (int i = 0; i < line.size(); i++) { row[i] = mapMap.at(line[i]); }
            rows++;
            map.push_back(row);
        } while (getline(fin, line));

        fin.close();
    }

    bool isAccessible(int r, int c) const {
        if (r < 0 || r >= rows || c < 0 || c >= cols) { return false; }
        return map[r][c] != 0 && map[r][c] != 1;
    }

    bool isNarrow(int r, int c) const {
        return isAccessible(r, c) && (isAccessible(r - 1, c) == isAccessible(r + 1, c)) &&
               (isAccessible(r, c - 1) == isAccessible(r, c + 1)) &&
               (!!isAccessible(r - 1, c - 1) || !isAccessible(r + 1, c + 1) || !isAccessible(r + 1, c - 1) ||
                isAccessible(r - 1, c + 1));
    }
};

struct Task {
    int agvs = 0;
    matrix<pii> positions;
    vector<int> tasks;

    Task(string input) {
        ifstream fin(input);
        if (!fin.is_open()) {
            DEBUG("Task file not found");
            exit(1);
        }

        fin >> agvs;
        positions.resize(agvs);
        int r, c, ts;
        for (int i = 0; i < agvs; i++) {
            fin >> ts;
            tasks.push_back(ts);
            positions[i].resize(ts);
            for (int j = 0; j < tasks[i]; j++) {
                fin >> r >> c;
                positions[i][j] = {r, c};
            }
        }

        fin.close();
    }
};

struct FloydMap {
    int rows = 0;
    int cols = 0;
    int size = 0;
    matrix<int> floyd;

    int toIndex(int r, int c) const { return r * cols + c; }
    pii toCoord(int index) const { return {index / cols, index % cols}; }

    FloydMap(const Map &map) {
        rows = map.rows;
        cols = map.cols;
        size = rows * cols;
        matrixResize(floyd, size, size, INF);

        // init
        for (int i = 0; i < size; i++) {
            auto [r, c] = toCoord(i);
            bool isAccessible = map.isAccessible(r, c);
            floyd[i][i] = (isAccessible ? 0 : INF);
            if (!isAccessible) { continue; }
            for (int j = 0; j < 4; j++) {
                int nr = r + dr[j];
                int nc = c + dc[j];
                if (map.isAccessible(nr, nc)) { floyd[i][toIndex(nr, nc)] = 1; }
            }
        }

        // floyd
        for (int k = 0; k < size; k++) {
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) { floyd[i][j] = min(floyd[i][j], floyd[i][k] + floyd[k][j]); }
            }
        }
    }

    int distance(int fromR, int fromC, int toR, int toC) const {
        return floyd[toIndex(fromR, fromC)][toIndex(toR, toC)];
    }
};

struct AStarNode {
    int r = -1;
    int c = -1;
    int dis = 0;
    int h = INF;

    AStarNode() = default;

    AStarNode(int r, int c, int dis, int h) : r(r), c(c), dis(dis), h(h) {}

    int f() const { return dis + h; }

    bool operator>(const AStarNode &other) const { return f() > other.f(); }
};

struct Conflict {
    int r = -1;
    int c = -1;
    int timestamp = -1;
    pii id = {-1, -1};

    Conflict() = default;
    Conflict(int r, int c, int timestamp, pii id) : r(r), c(c), timestamp(timestamp), id(id) {}
};

struct Constraint {
    int r = -1;
    int c = -1;
    int id = -1;
    int timestamp = -1;

    Constraint() = default;
    Constraint(int r, int c, int id, int timestamp) : r(r), c(c), id(id), timestamp(timestamp) {}
};

struct CbsNode {
    matrix<pii> paths;
    vector<Constraint> constraints;
    int cost = 0;

    CbsNode() = default;
    CbsNode(const matrix<pii> &paths, const vector<Constraint> &constraints, int cost)
        : paths(paths), constraints(constraints), cost(cost) {}

    bool operator>(const CbsNode &other) const { return cost > other.cost; }
};

// functions

/**
 * output file format:
 * agvs
 * tasks pos_0_r pos_0_c pos_1_r pos_1_c ...
 * tasks pos_0_r pos_0_c pos_1_r pos_1_c ... // for tasks * 2 numbers
 * ... // for agvs line
 */
void generateTasks(const Map &map, string output, int agvs, int tasks) {
    ofstream fout(output);

    uniform_int_distribution<int> distTask(0, max(1LL, (int)log2(tasks)));
    uniform_int_distribution<int> randomRow(0, map.rows - 1);
    uniform_int_distribution<int> randomCol(0, map.cols - 1);

    vector<pii> positions;
    for (int i = 0; i < map.rows; i++) {
        for (int j = 0; j < map.cols; j++) {
            if (map.isAccessible(i, j) && map.isNarrow(i, j)) { positions.emplace_back(i, j); }
        }
    }
    shuffle(positions.begin(), positions.end(), mt);

    vector<pii> initPos(positions.begin(), positions.begin() + agvs);

    auto randomPos = [&]() {
        int r = randomRow(mt);
        int c = randomCol(mt);
        while (!map.isAccessible(r, c) || map.isNarrow(r, c)) {
            r = randomRow(mt);
            c = randomCol(mt);
        }
        return pii(r, c);
    };

    auto p = positions.begin();

    fout << agvs << endl;
    for (int i = 0; i < agvs; i++) {
        int taskNumber = tasks - distTask(mt);
        auto [initR, initC] = randomPos();
        fout << taskNumber << " " << initR << " " << initC << " ";
        for (int j = 0; j < taskNumber - 1; j++) {
            auto [r, c] = *p;
            fout << r << " " << c << " ";

            p++;
            if (p == positions.end()) {
                shuffle(positions.begin(), positions.end(), mt);
                p = positions.begin();
            }
        }
        fout << endl;
    }

    fout.close();
}

int evaluateFloyd(const FloydMap &floyd, const Task &task) {
    int totalDistance = 0;
    for (int i = 0; i < task.agvs; i++) {
        for (int j = 0; j < task.tasks[i] - 1; j++) {
            auto [r1, c1] = task.positions[i][j];
            auto [r2, c2] = task.positions[i][j + 1];
            totalDistance += floyd.distance(r1, c1, r2, c2);
        }
    }
    return totalDistance;
}

int totalCost(const matrix<pii> &paths) {
    int ret = 0;
    for (auto path : paths) { ret += path.size(); }
    return ret;
}

int maxCost(const matrix<pii> &paths) { return std::ranges::max(paths, {}, &vector<pii>::size).size(); }

Conflict firstConflict(const matrix<pii> &paths) {
    int maxx = std::ranges::max(paths, {}, &vector<pii>::size).size();

    for (int step = 0; step < maxx; step++) {
        unordered_map<pii, int> positions;

        // point conflict
        for (int i = 0; i < paths.size(); i++) {
            if (step >= paths[i].size()) { continue; }
            pii pos = paths[i][step];
            if (positions.contains(pos)) {
                int conflict = positions[pos];
                return Conflict(pos.first, pos.second, step, {i, conflict});
            }
            positions[pos] = i;
        }

        // edge conflict
        if (step > 0) {
            for (int i = 0; i < paths.size(); i++) {
                if (step >= paths[i].size()) continue;
                for (int j = i + 1; j < paths.size(); j++) {
                    if (step >= paths[j].size()) continue;

                    pii pos1_t1 = paths[i][step];
                    pii pos1_t0 = paths[i][step - 1];
                    pii pos2_t1 = paths[j][step];
                    pii pos2_t0 = paths[j][step - 1];

                    // swap conflict
                    if (pos1_t1 == pos2_t0 && pos1_t0 == pos2_t1) {
                        return Conflict(pos1_t0.first, pos1_t0.second, step, {i, j});
                    }

                    // deadlock
                    if (pos1_t1 == pos2_t1 && pos1_t0 != pos2_t0) {
                        return Conflict(pos1_t1.first, pos1_t1.second, step, {i, j});
                    }
                }
            }
        }
    }

    return Conflict();
}

bool violateConstraint(int r, int c, int timestep, int id, const vector<Constraint> &constraints) {
    for (auto [cr, cc, cid, ctime] : constraints) {
        if (cr == r && cc == c && cid == id && ctime == timestep) { return true; }
    }
    return false;
}

int manhattan(int r1, int c1, int r2, int c2) { return abs(r1 - r2) + abs(c1 - c2); }

vector<pii> aStar(const Map &map, const FloydMap &floyd, int fromR, int fromC, int toR, int toC,
                  const vector<Constraint> &constraints, int id) {
    priority_queue<AStarNode, vector<AStarNode>, greater<AStarNode>> heap;
    vector<int> vis(floyd.size, false);
    vector<int> prev(floyd.size, -1);

    heap.push(AStarNode(fromR, fromC, 0, floyd.distance(fromR, fromC, toR, toC)));

    while (!heap.empty()) {
        auto cur = heap.top();
        heap.pop();

        int index = floyd.toIndex(cur.r, cur.c);
        if (cur.r == toR && cur.c == toC) {
            vector<pii> path;
            for (int p = index; prev[p] != -1; p = prev[p]) { path.push_back(map.toCoord(p)); }
            reverse(path);
            return path;
        }

        if (vis[index]) { continue; }
        // vis[index] = true;

        for (int i = 0; i < 4; i++) {
            int nr = cur.r + dr[i];
            int nc = cur.c + dc[i];
            int ntime = cur.dis + 1;

            if (violateConstraint(nr, nc, ntime, id, constraints)) { continue; }
            if (vis[map.toIndex(nr, nc)]) { continue; }

            if (map.isAccessible(nr, nc)) {
                int h = floyd.distance(nr, nc, toR, toC);
                heap.push(AStarNode(nr, nc, cur.dis + 1, h));
                prev[map.toIndex(nr, nc)] = index;
            }
        }
    }

    return {};
}

vector<pii> aStars(const Map &map, const FloydMap &floyd, const vector<pii> &targets,
                   const vector<Constraint> &constraints, int id) {
    vector<pii> path;

    for (int i = 0; i < targets.size() - 1; i++) {
        auto [fromR, fromC] = targets[i];
        auto [toR, toC] = targets[i + 1];
        auto subPath = aStar(map, floyd, fromR, fromC, toR, toC, constraints, id);
        if (subPath.empty()) { return {}; }
        path.insert(path.end(), subPath.begin(), subPath.end());
    }

    return path;
}

matrix<pii> cbs(const Map &map, const FloydMap &floyd, const Task &task) {
    priority_queue<CbsNode, vector<CbsNode>, greater<CbsNode>> heap;

    matrix<pii> paths(task.agvs);

    // search initial path
    for (int id = 0; id < task.agvs; id++) {
        paths[id] = aStars(map, floyd, task.positions[id], {}, id);
        if (paths[id].empty()) { return {}; }
    }

    heap.push(CbsNode(paths, {}, maxCost(paths)));
    while (!heap.empty()) {
        auto cur = heap.top();
        heap.pop();

        auto conflict = firstConflict(cur.paths);
        if (conflict.r == -1 && conflict.c == -1) { return cur.paths; }

        for (int i = 0; i < 2; i++) {
            int id = ((i == 0) ? conflict.id.first : conflict.id.second);

            auto newConstraints = cur.constraints;
            newConstraints.push_back(Constraint(conflict.r, conflict.c, id, conflict.timestamp));
            // lock the whole channel
            if (map.isNarrow(conflict.r, conflict.c) && i == 0) {
                queue<int> q;
                vector<int> vis;
                auto viss = [&](int r, int c) {
                    for (auto v : vis) {
                        if (map.toIndex(r, c) == v) { return true; }
                    }
                    return false;
                };

                q.push(map.toIndex(conflict.r, conflict.c));
                vis.push_back(map.toIndex(conflict.r, conflict.c));
                while (!q.empty()) {
                    int index = q.front();
                    q.pop();
                    auto [r, c] = map.toCoord(index);
                    for (int j = 0; j < 4; j++) {
                        int nr = r + dr[j];
                        int nc = c + dc[j];
                        if (!map.isAccessible(nr, nc)) { continue; }
                        if (map.isNarrow(nr, nc) && !viss(nr, nc)) {
                            q.push(map.toIndex(nr, nc));
                            vis.push_back(map.toIndex(nr, nc));
                            newConstraints.push_back(
                                Constraint(nr, nc, id, conflict.timestamp + manhattan(r, c, nr, nc)));
                        }
                    }
                }
            }

            matrix<pii> newPaths = cur.paths;
            newPaths[id] = aStars(map, floyd, task.positions[id], newConstraints, id);
            if (newPaths[id].empty()) { continue; }

            int newCost = maxCost(newPaths);
            heap.push(CbsNode(newPaths, newConstraints, newCost));
        }
    }

    return {};
}

// main

signed main() {

    // init
    Map map("map.txt");
    map.read();
    matrixPrint(map.map);
    DEBUG(map.rows);
    DEBUG(map.cols);

    int agvNum, agvTask;
    std::cin >> agvNum >> agvTask;

    generateTasks(map, "tasks.txt", agvNum, agvTask);
    Task task("tasks.txt");
    DEBUG(task.agvs);
    for (auto agv : task.positions) {
        for (auto pos : agv) { cerr << pos.first << " " << pos.second << " "; }
        cerr << endl;
    }


    // test
    Clock floydClock;
    FloydMap floyd(map);

    floydClock.start();
    int floydDistance = evaluateFloyd(floyd, task);
    floydClock.stop();

    DEBUG(floydDistance);
    DEBUG(floydClock.durationMus());

    Clock CbsClock;
    CbsClock.start();
    auto paths = cbs(map, floyd, task);
    CbsClock.stop();

    DEBUG(totalCost(paths));
    DEBUG(CbsClock.durationMus());




    return 0;
}