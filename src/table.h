#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "column.h"

class Table {

public:

    template <typename T>
    std::size_t addColumn(std::shared_ptr<Column<T>> &col);
    std::shared_ptr<AbstractColumn> column(std::size_t columnIndex) { return m_columns[columnIndex]; }

protected:
    std::vector<std::shared_ptr<AbstractColumn>> m_columns;

};

template <typename T>
std::size_t Table::addColumn(std::shared_ptr<Column<T>> &col) {
    m_columns.emplace_back(col);
    return m_columns.size() - 1;
}
