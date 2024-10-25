// ga

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>
#include <unordered_map>
#include <vector>

using namespace std;


#define int long long

template <typename T> using matrix = vector<vector<T>>;

const string input = "large_1000_gradian";
const string input_file = input + ".in";
const string output_file = input + "_ga.out";


const int _ms = 1;
const int _s = 1000 * _ms;
const int _m = 60 * _s;
const int _h = 60 * _m;
const int evaluateTime = 180 * _s;

const int populationSize = 500;
const int generations = 3500;
const double rate = 0.2;
const int tournamentSize = 5;
const int elitismSize = 10;

random_device rd;
mt19937 mt(rd());

vector<int> w;


int evaluate(const vector<int> &w, const vector<int> &p) {
    int n = w.size() - 2;
    int ret = 0;
    for (int i = 1; i <= n; i++) { ret += p[i] * w[p[i - 1]] * w[p[i]] * w[p[i + 1]]; }
    return ret;
}

vector<int> generateIndividual(int size) {
    vector<int> individual(size + 2);
    iota(individual.begin() + 1, individual.end() - 1, 1);
    shuffle(individual.begin() + 1, individual.end() - 1, mt);
    individual[0] = individual[size];
    individual[size + 1] = individual[1];
    return individual;
}

matrix<int> generatePopulation(int size, int populationSize) {
    matrix<int> population(populationSize, vector<int>(size + 2));
    vector<int> p(size + 2);
    iota(p.begin() + 1, p.end() - 1, 1);

    for (auto &individual : population) {
        shuffle(p.begin() + 1, p.end() - 1, mt);
        individual = p;
        individual[0] = individual[size];
        individual[size + 1] = individual[1];
    }
    return population;
}

vector<int> tournament(const matrix<int> &population, const vector<int> &fitnesses, int tournamentSize) {
    auto dist = uniform_int_distribution<int>(0, population.size() - 1);
    auto rand = [&]() { return dist(mt); };

    vector<int> tournament(tournamentSize);
    for (int &t : tournament) { t = rand(); }

    int index = tournament[0];
    for (int i : tournament) {
        if (fitnesses[i] > fitnesses[index]) { index = i; }
    }
    return population[index];
}

vector<int> PMX(const vector<int> &parent0, const vector<int> &parent1) {
    int size = parent0.size() - 2;
    vector<int> child(size + 2, -1);
    auto dist = uniform_int_distribution<int>(1, size);
    auto rand = [&]() { return dist(mt); };

    int from = rand(), to = rand();
    while (from == to) { to = rand(); }
    if (from > to) { swap(from, to); }
    for (int i = from; i <= to; i++) { child[i] = parent0[i]; }

    unordered_map<int, int> mapping;
    for (int i = from; i <= to; i++) { mapping[parent0[i]] = parent1[i]; }

    for (int i = 1; i <= size; i++) {
        if (child[i] == -1) {
            int gene = parent1[i];
            while (mapping.find(gene) != mapping.end()) { gene = mapping[gene]; }
            child[i] = gene;
        }
    }

    child[0] = child[size];
    child[size + 1] = child[1];
    return child;
}

void inverseMutation(vector<int> &individual) {
    int size = individual.size() - 2;
    auto dist = uniform_int_distribution<int>(1, size);
    auto rand = [&]() { return dist(mt); };
    int from = rand(), to = rand();
    while (from == to) { to = rand(); }
    if (from > to) { swap(from, to); }

    reverse(individual.begin() + from, individual.begin() + to);
    individual[0] = individual[size];
    individual[size + 1] = individual[1];
}

void swapMutation(vector<int> &individual) {
    int size = individual.size() - 2;
    auto dist = uniform_int_distribution<int>(1, size);
    auto rand = [&]() { return dist(mt); };
    int from = rand(), to = rand();
    while (from == to) { to = rand(); }
    swap(individual[from], individual[to]);
    individual[0] = individual[size];
    individual[size + 1] = individual[1];
}

vector<int> elitism(const matrix<int> population, const vector<int> fitnesses) {
    int index = distance(fitnesses.begin(), max_element(fitnesses.begin(), fitnesses.end()));
    return population[index];
}

matrix<int> elitismK(const matrix<int> population, const vector<int> fitnesses, int size) {
    vector<pair<int, int>> fs;
    for (int i = 0; i < population.size(); i++) { fs.emplace_back(fitnesses[i], i); }
    sort(fs.begin(), fs.end(), greater<pair<int, int>>());
    matrix<int> ret;
    for (int i = 0; i < size && i < fitnesses.size(); i++) { ret.push_back(population[fs[i].second]); }
    return ret;
}

vector<int> geneticAlgorithm(const vector<int> &w, int populationSize, int generations, double rate, int tournamentSize,
                             int elitismSize) {
    int n = w.size() - 2;
    auto population = generatePopulation(n, populationSize);
    auto bestIndividual = population[0];
    int bestFitness = evaluate(w, bestIndividual);
    int generation = 0;

    auto distReal = uniform_real_distribution<double>(0, 1);
    auto randomReal = [&]() { return distReal(mt); };
    auto dist01 = uniform_int_distribution<int>(0, 1);
    auto random01 = [&]() { return dist01(mt); };

    auto start = chrono::high_resolution_clock::now();
    auto duration = [&]() {
        return chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count();
    };

    while (duration() < evaluateTime) {
        generation += 1;

        int bestFitnessIndex = -1;
        vector<int> fitnesses(populationSize);
        for (int i = 0; i < populationSize; i++) {
            int fitness = evaluate(w, population[i]);
            fitnesses[i] = fitness;
            if (fitness > bestFitness) {
                bestFitness = fitness;
                bestFitnessIndex = i;
            }
        }
        if (bestFitnessIndex != -1) { bestIndividual = population[bestFitnessIndex]; }

        matrix<int> newPopulation = elitismK(population, fitnesses, elitismSize);

        double mutationRate = rate * (1.0 - (double)generation / generations);
        while (newPopulation.size() < populationSize) {

            auto parent0 = tournament(population, fitnesses, tournamentSize);
            auto parent1 = tournament(population, fitnesses, tournamentSize);

            vector<int> child;
            child = PMX(parent0, parent1);


            if (randomReal() < mutationRate) {
                if (random01()) {
                    inverseMutation(child);
                } else {
                    swapMutation(child);
                }
            }

            newPopulation.push_back(child);
        }

        population = newPopulation;
    }

    return bestIndividual;
}

signed main() {

    freopen(input_file.c_str(), "r", stdin);
    freopen(output_file.c_str(), "w", stdout);

    // input
    int n;
    cin >> n;
    w.resize(n + 2);
    for (int i = 1; i <= n; i++) { cin >> w[i]; }

    // ga

    auto startTime = chrono::high_resolution_clock::now();
    auto solution = geneticAlgorithm(w, populationSize, generations, rate, tournamentSize, elitismSize);

    cout << "ans: " << evaluate(w, solution) << endl;
    cout << "duration: "
         << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - startTime).count()
         << "ms" << endl;
    for (int i = 1; i <= n; i++) { cout << solution[i] << " "; }



    return 0;
}