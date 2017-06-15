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
    ColumnPtr column(std::size_t columnIndex) { return m_columns[columnIndex]; }
    std::vector<ColumnPtr> getColumns(std::vector<std::size_t> &columnIndcies);

protected:
    std::vector<ColumnPtr> m_columns;

};

template <typename T>
std::size_t Table::addColumn(std::shared_ptr<Column<T>> &col) {
    m_columns.emplace_back(col);
    return m_columns.size() - 1;
}

std::vector<ColumnPtr> Table::getColumns(std::vector<std::size_t> &columnIndcies) {

    std::vector<ColumnPtr> cols(columnIndcies.size());

    for (uint i = 0; i < columnIndcies.size(); ++i) 
    {
        cols.at(i) = this->column(columnIndcies.at(i));
    }

    return cols;
}
