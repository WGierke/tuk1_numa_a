#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "column.h"

typedef std::shared_ptr<AbstractColumn> ColumnPtr;

class Table {

public:

    template <typename T>
    std::size_t addColumn(std::shared_ptr<Column<T>> &col);
    ColumnPtr column(std::size_t columnIndex);
    std::vector<ColumnPtr> getColumns(std::vector<std::size_t> &columnIndcies);
    std::vector<ColumnPtr> getColumns();

protected:
    std::vector<ColumnPtr> m_columns;

};

template <typename T>
std::size_t Table::addColumn(std::shared_ptr<Column<T>> &col) {
    m_columns.emplace_back(col);
    return m_columns.size() - 1;
}
