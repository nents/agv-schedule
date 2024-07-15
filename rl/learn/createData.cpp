#include <iostream>
#include <random>

using namespace std;

mt19937 gen;

int main() {

    gen.seed(random_device()());


    return 0;
}