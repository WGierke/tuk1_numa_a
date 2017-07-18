#include "table.h"

#include <unordered_map>
#include <limits>

template <typename TPredicate>
Table Table::where(int col, const TPredicate &pred)
{
    std::vector<std::vector<uint32_t, typename Column<uint32_t>::allocator_t>> result;
    auto materializer = [&](std::vector<size_t> &columns, std::size_t row) {

        auto allocateColumns = columns;
        if (allocateColumns.size() && allocateColumns[0] == std::numeric_limits<size_t>::max())
        {
            allocateColumns.clear();
            for (size_t i = 0; i < m_columns.size(); ++i)
                allocateColumns.emplace_back(i);
        }

        // Initialize result columns if necessary (first row)
        if (result.size() != allocateColumns.size())
        {
            for (auto &column : allocateColumns)
            {
                // Allocate result column on the same node as source column
                auto allocator = Column<uint32_t>::allocator_t(m_columns[column]->numaNode());
                result.emplace_back(std::vector<uint32_t, typename Column<uint32_t>::allocator_t>(0, allocator));
            }
        }

        auto i = 0;
        for (auto &col : allocateColumns)
        {
            result[i++].emplace_back(m_columns[col]->valueAt(row));
        }
    };

    std::size_t row = 0;
    for (auto &attribute : m_columns[col]->data())
    {
        pred(attribute, materializer, row);
        ++row;
    }

    auto resultRows = result.size() > 0 ? result[0].size() : 0;
    Table resultTable(resultRows);
    for (auto &col : result)
    {
        auto c = std::make_shared<Column<uint32_t>>(std::move(col));
        resultTable.addColumn(c);
    }
    return resultTable;
}

template <typename TPredicate>
Table Table::hashJoinRows(size_t index, const std::vector<std::size_t> &rows, Table &other, size_t other_index, const TPredicate &pred) {
    std::vector<std::vector<uint32_t, typename Column<uint32_t>::allocator_t>> result;
    auto materializer = [&](std::vector<size_t> &columns, std::vector<size_t> &otherColumns, std::size_t row, std::size_t otherRow) {

        auto allocateColumns = columns;
        if (allocateColumns.size() && allocateColumns[0] == std::numeric_limits<size_t>::max())
        {
            allocateColumns.clear();
            for (size_t i = 0; i < m_columns.size(); ++i)
                allocateColumns.emplace_back(i);
        }

        auto allocateOtherColumns = otherColumns;
        if (allocateOtherColumns.size() && allocateOtherColumns[0] == std::numeric_limits<size_t>::max())
        {
            allocateOtherColumns.clear();
            for (size_t i = 0; i < other.m_columns.size(); ++i)
                allocateOtherColumns.emplace_back(i);
        }

        // Initialize result columns if necessary (first row)
        if (result.size() != allocateColumns.size() + allocateOtherColumns.size())
        {
            for (auto &column : allocateColumns)
            {
                // Allocate result column on the same node as source column
                auto allocator = Column<uint32_t>::allocator_t(m_columns[column]->numaNode());
                result.emplace_back(std::vector<uint32_t, typename Column<uint32_t>::allocator_t>(0, allocator));
            }

            for (auto &column : allocateOtherColumns)
            {
                auto allocator = Column<uint32_t>::allocator_t(other.m_columns[column]->numaNode());
                result.emplace_back(std::vector<uint32_t, typename Column<uint32_t>::allocator_t>(0, allocator));
            }
        }

        auto i = 0;
        for (auto &col : allocateColumns)
        {
            result[i++].emplace_back(m_columns[col]->valueAt(row));
        }

        for (auto &col : allocateOtherColumns)
        {
            result[i++].emplace_back(other.m_columns[col]->valueAt(row));
        }
    };

    auto &column = this->column(index)->data();
    auto &other_column = other.column(other_index)->data();

    // We weren't using multimaps before, so actually the hash join operation
    // wasn't implemented correctly. Definitely need them for Task 3, because we have multiple
    // rows with the same values
    using map_t = std::unordered_multimap<uint32_t, size_t>;

    // TODO: Also omitting allocating to bind the map onto a NUMA node -- was getting allocation failures
    // Now it's probably sliced across all nodes, which is not what we want
    map_t col_map;
    col_map.reserve(column.size() + 1);

    if (rows.size())
    {
        for (auto &selectedRow : rows)
        {
            col_map.emplace(column[selectedRow], selectedRow);
        }
    }
    else
    {
        for (size_t i = 0; i < column.size(); ++i)
        {
            col_map.emplace(column[i], i);
        }
    }

    for (size_t j = 0; j < other_column.size(); ++j)
    {
        auto val = other_column[j];
        auto partners = col_map.equal_range(val);

        for (auto it = partners.first; it != partners.second; ++it)
        {
            pred(it->second, j, materializer);
        }
    }

    auto resultRows = result.size() > 0 ? result[0].size() : 0;
    Table resultTable(resultRows);
    for (auto &col : result)
    {
        auto c = std::make_shared<Column<uint32_t>>(std::move(col));
        resultTable.addColumn(c);
    }
    return resultTable;
}
