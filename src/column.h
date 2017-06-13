#pragma once

#include <iostream>

#include "numaalloc.h"

class AbstractColumn {
public:
    virtual void scan() const = 0;
    virtual void scanRandom() const = 0;
};

template <typename T>
class Column : public AbstractColumn {
public:
    Column(std::size_t size, int node)
        : m_attributeVector(size, allocator_t(node)) {}

    typedef NumaAlloc<T> allocator_t;

    void scan() const override;
    void scanRandom() const override;
    std::vector<T, allocator_t> &data() { return m_attributeVector; }

protected:
    std::vector<T, allocator_t> m_attributeVector;
};

template<typename T>
void Column<T>::scan() const {
    T aggregate = 0;
    for (auto &attribute : m_attributeVector)
    {
        aggregate ^= attribute;
    }
    std::cout << aggregate;
}

template<typename T>
void Column<T>::scanRandom() const {
    T aggregate = 0;
    for (auto i = 0ul; i < m_attributeVector.size(); ++i)
    {
        aggregate ^= m_attributeVector[std::rand() % m_attributeVector.size()];
    }
    std::cout << aggregate;
}
