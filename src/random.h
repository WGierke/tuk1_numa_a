#pragma once
#include <random>

extern unsigned long x, y, z;

class Random {
public:
    static inline unsigned long next() {
        if (x == 0) x = rand();

        x ^= x << 16;
        x ^= x >> 5;
        x ^= x << 1;

        unsigned long t = x;
        x = y;
        y = z;
        z = t ^ x ^ y;
        return z;
    }
};
