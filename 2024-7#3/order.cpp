#include "order.h"


bool Order::input(string filename) {
    {
        // input order
        ifstream fin(filename);
        if (!fin) return false;
        int row, col;
        while (fin >> row >> col) { order.push_back({row, col}); }
        fin.close();
    }
    orders = order.size();
    return true;
}