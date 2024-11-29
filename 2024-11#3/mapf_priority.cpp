#include "mapf.hpp"

// constants

const int INF = 1e18;
const int dr[] = {0, 1, 0, -1};
const int dc[] = {1, 0, -1, 0};

const unordered_map<char, int> mapMap = {{'#', 0}, {'$', 1}, {'o', 2}, {'P', 3}};

const int AGV_SIZE = 10;
const int TASK_SIZE = 20;

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
               (isAccessible(r, c - 1) == isAccessible(r, c + 1));
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
    int index = -1;
    int dis = 0;
    int h = INF;

    AStarNode() = default;

    AStarNode(int index, int dis, int h) : index(index), dis(dis), h(h) {}

    int f() const { return dis + h; }

    bool operator>(const AStarNode &other) const {
        if (f() == other.f()) { return dis < other.dis; }
        return f() > other.f();
    }
};

struct AStarMap {
    int size = 0;
    vector<unordered_set<int>> locks;

    AStarMap() = default;
    AStarMap(int size) : size(size), locks(size) {}

    void lock(int index, int time) { locks[index].emplace(time); }

    bool isLocked(int index, int time) const {
        if (locks[index].empty()) { return false; }
        return locks[index].find(time) != locks[index].end();
    }
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

    auto randomPos = [&]() {
        int r = randomRow(mt);
        int c = randomCol(mt);
        while (!map.isAccessible(r, c) || map.isNarrow(r, c)) {
            r = randomRow(mt);
            c = randomCol(mt);
        }
        return pii(r, c);
    };

    set<pii> uniquePos;
    while (uniquePos.size() < agvs) { uniquePos.emplace(randomPos()); }
    vector<pii> initPos(uniquePos.begin(), uniquePos.end());

    fout << agvs << endl;
    for (int i = 0; i < agvs; i++) {
        int taskNumber = tasks - distTask(mt);
        fout << taskNumber << " " << initPos[i].first << " " << initPos[i].second << " ";
        for (int j = 0; j < taskNumber - 1; j++) {
            auto [r, c] = randomPos();
            fout << r << " " << c << " ";
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

template <typename T> int totalCost(const matrix<T> &m) {
    int ret = 0;
    for (auto path : m) { ret += path.size(); }
    return ret;
}

template <typename T> int maxCost(const matrix<T> &m) { return std::ranges::max(m, {}, &vector<T>::size).size(); }

vector<int> aStar(const Map &map, const FloydMap &floyd, AStarMap &aStarMap, int fromIndex, int toIndex,
                  int startTime) {
    priority_queue<AStarNode, vector<AStarNode>, greater<AStarNode>> heap;
    vector<int> vis(aStarMap.size, false);
    vector<int> prev(aStarMap.size, -1);

    auto dist = [&](int from, int to) {
        auto [r1, c1] = map.toCoord(from);
        auto [r2, c2] = map.toCoord(to);
        return floyd.distance(r1, c1, r2, c2);
    };

    heap.emplace(fromIndex, startTime, dist(fromIndex, toIndex));

    while (!heap.empty()) {
        auto cur = heap.top();
        int index = cur.index;
        auto [r, c] = map.toCoord(index);
        heap.pop();

        if (vis[index]) { continue; }
        vis[index] = true;

        if (index == toIndex) {
            vector<int> path;
            for (int p = index; prev[p] != -1; p = prev[p]) { path.push_back(p); }
            reverse(path);
            aStarMap.lock(fromIndex, startTime);
            int time = startTime + 1;
            for (int i : path) { aStarMap.lock(i, time++); }
            return path;
        }

        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i];
            int nc = c + dc[i];
            int ntime = cur.dis + 1;
            int nindex = map.toIndex(nr, nc);

            if (vis[nindex]) { continue; }
            if (!map.isAccessible(nr, nc)) { continue; }
            if (aStarMap.isLocked(nindex, ntime)) { continue; }

            int h = dist(nindex, toIndex);
            heap.emplace(nindex, ntime, h);
            prev[nindex] = index;
        }
    }

    return {};
}

vector<int> aStars(const Map &map, const FloydMap &floyd, AStarMap &aStarMap, const vector<pii> &targets) {
    vector<int> path;

    for (int i = 0; i < targets.size() - 1; i++) {
        int from = map.toIndex(targets[i].first, targets[i].second);
        int to = map.toIndex(targets[i + 1].first, targets[i + 1].second);
        auto subPath = aStar(map, floyd, aStarMap, from, to, path.size());
        if (subPath.empty()) { return {}; }
        path.insert(path.end(), subPath.begin(), subPath.end());
    }

    return path;
}

matrix<int> evaluateAll(const Map &map, const FloydMap &floyd, const Task &task) {
    matrix<int> paths(task.agvs);
    auto aStarMap = AStarMap(floyd.size);

    for (int i = 0; i < task.agvs; i++) {
        paths[i] = aStars(map, floyd, aStarMap, task.positions[i]);
        if (paths[i].empty()) { return {}; }
    }

    return paths;
}

// main

signed main() {

    int agvs = AGV_SIZE;
    int tasks = TASK_SIZE;

    // intput
    // cin >> agvs >> tasks;

    // init
    Map map("map.txt");
    map.read();
    matrixPrint(map.map);
    DEBUG(map.rows);
    DEBUG(map.cols);

    // generateTasks(map, "tasks.txt", agvs, tasks);
    Task task("tasks.txt");

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



    Clock aStarClock;
    aStarClock.start();
    auto paths = evaluateAll(map, floyd, task);
    aStarClock.stop();

    // DEBUG(AGV_SIZE);
    // DEBUG(TASK_SIZE);
    DEBUG(totalCost(paths));
    // DEBUG(maxCost(paths));
    DEBUG(aStarClock.durationMus());



    return 0;
}