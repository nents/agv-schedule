#include "greedy4simulate.hpp"
#include "sa4lowerbound.hpp"

int main() {
    DEBUG("main");

    sa4lowerbound();
    greedy4simulate();

    DEBUG("all end");
    return 0;
}