#include "table.h"

std::size_t Table::addColumn(ColumnPtr &col) {
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

std::vector<ColumnPtr> Table::getColumns() {
    std::vector<ColumnPtr> cols(m_columns.size());
    for (uint i = 0; i < m_columns.size(); ++i) {
        cols.at(i) = this->column(i);
    }
    return cols;
}

ColumnPtr Table::column(std::size_t columnIndex) {
    return m_columns[columnIndex];
}

std::vector<std::vector<uint32_t>> Table::scanRows(const std::vector<size_t> &indices) {
    std::vector<std::vector<uint32_t>> rows(indices.size(), std::vector<uint32_t>(m_columns.size()));
    for (uint i = 0; i < indices.size(); ++i)
    {
        for (uint j = 0; j < m_columns.size(); ++j)
        {
            rows[i][j] = m_columns[j]->valueAt(indices[i]);
        }
    }

    return rows;
}

std::vector<JoinResult> Table::join(size_t index, Table &other, size_t other_index) {
    std::vector<JoinResult> result;

    auto &column = this->column(index)->data();
    auto &other_column = other.column(other_index)->data();

    for (auto c_it = column.begin(); c_it != column.end(); ++c_it)
    {
        for (auto o_it = other_column.begin(); o_it != other_column.end(); ++o_it)
        {
            if (*c_it == *o_it) {
                size_t col_row = c_it - column.begin();
                size_t other_row = o_it - other_column.begin();
                JoinResult res = std::make_pair(col_row, other_row);
                result.push_back(res);
            }
        }
    }

    return result;
}

std::vector<JoinResult> Table::hashJoin(size_t index, Table &other, size_t other_index) {
    std::vector<JoinResult> result;

    auto &column = this->column(index)->data();
    auto &other_column = other.column(other_index)->data();

    using allocator_t = NumaAlloc<std::pair<const uint32_t, size_t>>;
    using map_t = std::unordered_map<uint32_t, size_t, std::hash<uint32_t>, std::equal_to<uint32_t>, allocator_t>;

    // Allocate map on second column (usually remote)
    // TODO: I think the first parameter shouldn't be 0 (hoping to let the map use its internal default)
    map_t col_map (0, std::hash<uint32_t>(), std::equal_to<uint32_t>(), allocator_t(other.column(other_index)->numaNode()));
    col_map.reserve(column.size() + 1);

    for (size_t i = 0; i < column.size(); ++i)
    {
        col_map.emplace(column[i], i);
    }

    for (size_t j = 0; j < other_column.size(); ++j)
    {
        auto val = other_column[j];
        auto partner = col_map.find(val);

        if (partner != col_map.end())
        {
            result.emplace_back(partner->second, j);
        }
    }

    return result;
}
