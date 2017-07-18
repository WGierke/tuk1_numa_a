#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "column.h"

typedef std::shared_ptr<Column<uint32_t>> ColumnPtr;
typedef std::pair<size_t, size_t> JoinResult;

class Table {

public:
    Table(size_t rows) : m_rows(rows) {}

    std::size_t addColumn(ColumnPtr &col);
    ColumnPtr column(std::size_t columnIndex);
    std::vector<ColumnPtr> getColumns(std::vector<std::size_t> &columnIndcies);
    std::vector<ColumnPtr> getColumns();
    std::vector<std::vector<uint32_t>> scanRows(const std::vector<size_t> &indices);
    std::vector<uint32_t> scanRow(size_t index);
    std::vector<JoinResult> join(size_t index, Table &other, size_t other_index);
    std::vector<JoinResult> hashJoin(size_t index, Table &other, size_t other_index);
    const size_t rows() { return m_rows; }

protected:
    std::vector<ColumnPtr> m_columns;
    size_t m_rows;

};
