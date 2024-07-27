// #include "top.h"

// using namespace std;

// int width, height;

// mt19937 gen;

// int main() {
//     gen.seed(random_device()());

//     ifstream fin(map_file);
//     if (!fin.is_open()) {
//         cout << "Unable to open file" << endl;
//         return 1;
//     }

//     matrix<char> mapData;
//     string line;
//     while (getline(fin, line)) {
//         vector<char> row;
//         for (char c : line) {
//             if (c == '\r' || c == '\n') break;
//             row.push_back(c);
//         }
//         mapData.push_back(row);
//         width = max(width, (int)row.size());
//         height++;
//     }
//     fin.close();

//     for (int i = 0; i < height; i++) {
//         for (int j = 0; j < width; j++) { cout << mapData[i][j]; }
//         cout << endl;
//     }

//     uniform_int_distribution<int> randRow(0, height - 1);
//     uniform_int_distribution<int> randCol(0, width - 1);

//     ofstream fout(order_file);
//     for (int i = 0; i < MAX_ORDER; i++) {
//         int row = randRow(gen);
//         int col = randCol(gen);
//         while (mapData[row][col] != 'X') {
//             row = randRow(gen);
//             col = randCol(gen);
//         }

//         fout << row << " " << col << endl;
//     }
//     fout.close();

//     return 0;
// }
