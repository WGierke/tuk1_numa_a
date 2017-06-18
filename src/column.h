#pragma once

#include <iostream>

#include "numaalloc.h"

class AbstractColumn {
public:
    virtual void scan() const = 0;
};

template <typename T>
class Column : public AbstractColumn {
public:
    Column(std::size_t size, int node)
        : m_attributeVector(size, allocator_t(node)) {}

    typedef NumaAlloc<T> allocator_t;

    void scan() const override;

    std::vector<T, allocator_t> &data() { return m_attributeVector; }

protected:
    std::vector<T, allocator_t> m_attributeVector;
};

// fast random number generator
inline unsigned long fastrandom(unsigned long &x, unsigned long &y, unsigned long &z) {
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    unsigned long t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;
    return z;
}

template<typename T>
void Column<T>::scan() const {
    T aggregate = 0;
    for (auto &attribute : m_attributeVector)
    {
        aggregate ^= attribute;
    }
    std::cout << aggregate;
}
