#include "sa4lowerbound.h"

namespace {
int argAdjMin(ref<Graph> G, int fromR, int fromC, int toR, int toC) {
    int mn = INT_SOFT_MAX, argMn = 0;
    for (var i = 0; i < 4; i++) {
        int tr = toR + dr[i], tc = toC + dc[i];
        if (tr < 0 || tr >= G.rows || tc < 0 || tc >= G.cols) continue;
        int distance = G.shortestPath(fromR, fromC, tr, tc);
        if (distance < mn) {
            mn = distance;
            argMn = i;
        }
    }
    return argMn;
}

double dp(ref<Graph> G, ref<Order> O, ref<vector<int>> schedule) {
    fun cost = [&](ref<vector<pii>> task, int from, int to) {
        var ret = 0.0;
        var lastR = sendArea[0];
        var lastC = sendArea[1];
        for (var i = from; i <= to; i++) {
            val r = task[i][0], c = task[i][1];
            val argMn = argAdjMin(G, lastR, lastC, r, c);
            ret += G.shortestPath(lastR, lastC, r + dr[argMn], c + dc[argMn]);
            lastR = r + dr[argMn];
            lastC = c + dc[argMn];
        }
        ret += G.shortestPath(lastR, lastC, sendArea[0], sendArea[1]);
        return ret;
    };
    //
    var ret = 0;
    for (var idx = 0; idx < MAX_AGV; idx++) {
        vector<pii> task;
        for (var i = 0; i < schedule.size(); i++) {
            val agv = schedule[i];
            if (agv == idx) task.push_back(O.order[i]);
        }
        if (task.empty()) continue;
        // dp
        vector<double> f(task.size(), INT_SOFT_MAX);
        for (int i = 0; i < 4; i++) {
            int tr = task[0][0] + dr[i], tc = task[0][1] + dc[i];
            f[0] = min(f[0], G.shortestPath(agvRestArea[idx][0], agvRestArea[idx][1], tr, tc) +
                                 G.shortestPath(tr, tc, sendArea[0], sendArea[1]) * 1.0);
        }
        for (var i = 1; i < task.size(); i++) {
            for (var j = i; j > 0 && i - j < MAX_AGV_TASK; j--) {
                double c = cost(task, j, i);
                f[i] = min(f[i], f[j - 1] + c);
            }
        }
        ret += f.back();
    }
    return ret;
}

void convert2path(ref<Graph> G, ref<Order> O, ref<vector<int>> schedule) {
    fun cost = [&](ref<vector<pii>> task, int from, int to) {
        var ret = 0.0;
        var lastR = sendArea[0];
        var lastC = sendArea[1];
        for (var i = from; i <= to; i++) {
            val r = task[i][0], c = task[i][1];
            val argMn = argAdjMin(G, lastR, lastC, r, c);
            ret += G.shortestPath(lastR, lastC, r + dr[argMn], c + dc[argMn]);
            lastR = r + dr[argMn];
            lastC = c + dc[argMn];
        }
        ret += G.shortestPath(lastR, lastC, sendArea[0], sendArea[1]);
        return ret;
    };
    fun getPath = [&](pii from, pii to) {
        val argMn = argAdjMin(G, from[0], from[1], to[0], to[1]);
        val goalR = to[0] + dr[argMn], goalC = to[1] + dc[argMn];
        return make_pair(pii{goalR, goalC}, G.traceSimplePath(from[0], from[1], goalR, goalC));
    };
    // create a empty file
    {
        ofstream fout(sa4lowerbound_path_file);
        if (!fout) DEBUG("create file error");
        fout.close();
    }
    //
    ofstream fout(sa4lowerbound_path_file, std::ios::app);
    for (var idx = 0; idx < MAX_AGV; idx++) {
        vector<pii> task;
        for (var i = 0; i < schedule.size(); i++) {
            val agv = schedule[i];
            if (agv == idx) task.push_back(O.order[i]);
        }
        if (task.empty()) continue;
        // dp
        vector<double> f(task.size(), INT_SOFT_MAX);
        vector<int> last(task.size(), 0);
        for (int i = 0; i < 4; i++) {
            int tr = task[0][0] + dr[i], tc = task[0][1] + dc[i];
            f[0] = min(f[0], G.shortestPath(agvRestArea[idx][0], agvRestArea[idx][1], tr, tc) +
                                 G.shortestPath(tr, tc, sendArea[0], sendArea[1]) * 1.0);
        }
        for (var i = 1; i < task.size(); i++) {
            for (var j = i; j > 0 && i - j < MAX_AGV_TASK; j--) {
                double c = cost(task, j, i);
                // f[i] = min(f[i], f[j - 1] + c);
                if (f[i] > f[j - 1] + c) {
                    f[i] = f[j - 1] + c;
                    last[i] = j - 1;
                }
            }
        }
        // convert task to route
        vector<pii> route;
        {
            int p = last.back(), head = task.size() - 1;
            while (p) {
                route.push_back(sendArea);
                while (head > p) { route.push_back(task[head--]); }
                route.push_back(sendArea);
                head = p;
                p = last[p];
            }
            while (head >= 0) { route.push_back(task[head--]); }
            route.push_back(agvRestArea[idx]);
        }
        reverse(route.begin(), route.end());
        // convert route to path
        vector<pii> path;
        {
            var last = route.front();
            for (var i = 1; i < route.size(); i++) {
                val now = route[i];
                pair<pii, vector<pii>> ret = getPath(last, now);
                last = ret.first;
                path.insert(path.end(), ret.second.begin(), ret.second.end());
            }
        }
        // output
        for (val v : path) { fout << v[0] << " " << v[1] << " "; }
        fout << endl;
    }
    fout.close();
}
} // namespace

void sa4lowerbound() {
    DEBUG("begin sa4lowerbound");

    // constants
    const int SA_MAX_ITER = 1;
    const double SA_INIT_T = 5000.0;
    const double eps = 1e-9;
    const double SA_DELTA_T = 0.997;

    // variables
    mt19937 mt(random_device{}());
    Graph G;
    Order O;

    // init
    G.input(map_file);
    G.solveShortestPath();
    O.input(order_file);

    // sa
    fun evaluate = [&](ref<vector<int>> schedule) { return dp(G, O, schedule); };
    uniform_int_distribution<int> randAgv(0, MAX_AGV - 1);
    uniform_int_distribution<int> randOrder(0, O.orders - 1);
    uniform_real_distribution<double> rand01(0, 1);

    vector<int> schedule(O.orders);
    var gBestSchedule = schedule;
    var gBsetAns = INT_SOFT_MAX;
    DEBUG("asd");
    for (int times = 0; times < SA_MAX_ITER; times++) {
        var t = SA_INIT_T;
        for (int &v : schedule) { v = randAgv(mt); }
        var bestAns = evaluate(schedule);

        while (t > eps) {
            val pos = randOrder(mt);
            val redo = schedule[pos];
            schedule[pos] = randAgv(mt);
            val nowAns = evaluate(schedule);
            val delta = nowAns - bestAns;
            if (delta < 0 || exp(-delta / t) > rand01(mt)) {
                bestAns = nowAns;
                if (nowAns < gBsetAns) {
                    gBsetAns = nowAns;
                    gBestSchedule = schedule;
                }
            } else {
                schedule[pos] = redo;
            }
            t *= SA_DELTA_T;
        }
    }

    DEBUG(gBsetAns);
    DEBUG("schedule: ");
    for (var v : gBestSchedule) { cerr << v << " "; }
    cerr << endl;
    // output
    ofstream fout(sa4lowerbound_file);
    for (var v : gBestSchedule) { fout << v << " "; }
    fout.close();
    DEBUG("output path");
    convert2path(G, O, gBestSchedule);

    DEBUG("end sa4lowerbound");
}