#pragma once

#include <iostream>
#include <vector>

#include "numaalloc.h"

class AbstractColumn {
public:
    virtual void scan() const = 0;
};

template <typename T>
class Column : public AbstractColumn {
public:
    typedef NumaAlloc<T> allocator_t;

    Column(std::size_t size, int node) : m_attributeVector(size, allocator_t(node)) {}
    void scan() const override;
    std::vector<T, allocator_t> &data() { return m_attributeVector; }

protected:
    std::vector<T, allocator_t> m_attributeVector;
};

template <class T>
void doNotOptimizeAway(T&& datum) {
  // Facebook style
  // https://stackoverflow.com/questions/40122141/preventing-compiler-optimizations-while-benchmarking
  asm volatile("" : "+r" (datum));
}

template<typename T>
void Column<T>::scan() const {
    T aggregate = 0;
    for (auto &attribute : m_attributeVector)
    {
        aggregate ^= attribute;
    }
    doNotOptimizeAway(aggregate);
}
