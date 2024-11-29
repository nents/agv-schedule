#include <iostream>
#include <vector>

using namespace std;


int main() {

    cout << sizeof(size_t) << endl;
    cout << sizeof(void *) << endl;

    int index = 0;
    vector<int> vec;
    while (1) {
        cout << index << " -> " << vec.capacity() << endl;
        index++;
        for (int i = 0; i < 1024; i++) { vec.push_back(i); }
    }


    return 0;
}