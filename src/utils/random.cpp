#include "random.h"
#include <chrono>

std::mt19937 Random::generator;

void Random::initialize() {
    generator.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
}

int Random::getInt(int min, int max) {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

double Random::getDouble(double min, double max) {
    std::uniform_real_distribution<double> distribution(min, max);
    return distribution(generator);
}
