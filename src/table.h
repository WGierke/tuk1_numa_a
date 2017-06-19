#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "column.h"

typedef std::shared_ptr<Column<uint32_t>> ColumnPtr;

class Table {

public:

    std::size_t addColumn(ColumnPtr &col);
    ColumnPtr column(std::size_t columnIndex);
    std::vector<ColumnPtr> getColumns(std::vector<std::size_t> &columnIndcies);
    std::vector<ColumnPtr> getColumns();
    std::vector<std::vector<uint32_t>> scanRows(const std::vector<size_t> &indices);
    std::vector<uint32_t> scanRow(size_t index);

protected:
    std::vector<ColumnPtr> m_columns;

};
