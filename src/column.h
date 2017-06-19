#pragma once

#include <iostream>
#include <vector>

#include "numaalloc.h"

template <typename T>
class Column {
public:
    typedef NumaAlloc<T> allocator_t;

    Column(std::size_t size, int node) : m_attributeVector(size, allocator_t(node)) {}
    void scan() const;
    std::vector<T, allocator_t> &data() { return m_attributeVector; }
    T valueAt(size_t index) { return m_attributeVector.at(index); }

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
