#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <functional>

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
    template <typename TPredicate>
    Table hashJoinRows(size_t index, const std::vector<std::size_t> &rows, Table &other, size_t other_index, const TPredicate &pred);
    template <typename TPredicate>
    Table where(int col, const TPredicate &pred);
    const size_t rows() { return m_rows; }

    typedef std::function<void(std::vector<size_t> &, std::size_t)> materializer;
    typedef std::function<void(std::vector<size_t> &, std::vector<size_t> &, std::size_t, std::size_t)> joinMaterializer;

protected:
    std::vector<ColumnPtr> m_columns;
    size_t m_rows;

};

#include "table.hpp"
