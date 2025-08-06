#ifndef RANDOM_H
#define RANDOM_H

#include <random>

class Random {
public:
    static void initialize();
    static int getInt(int min, int max);
    static double getDouble(double min, double max);

private:
    static std::mt19937 generator;
};

#endif // RANDOM_H
