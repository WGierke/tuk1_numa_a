#include <unordered_map>

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

std::vector<uint32_t> Table::scanRow(size_t index) {
    std::vector<uint32_t> row(m_columns.size());
    for (uint i = 0; i < m_columns.size(); ++i)
    {
        row.at(i) = this->column(i)->valueAt(index);
    }

    return row;
}

std::vector<std::vector<uint32_t>> Table::scanRows(std::vector<size_t> indices) {
    std::vector<std::vector<uint32_t>> rows(indices.size(), std::vector<uint32_t>(m_columns.size()));
    for (uint i = 0; i < indices.size(); ++i)
    {
        rows.at(i) = this->scanRow(i);
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

    std::unordered_map<uint32_t, size_t> col_map;
    col_map.reserve(column.size() + 1);

    for (size_t i = 0; i < column.size(); ++i)
    {
        col_map.insert(std::make_pair(column.at(i), i));
    }

    for (size_t j = 0; j < other_column.size(); ++j)
    {
        auto val = other_column.at(j);
        auto partner = col_map.find(val);

        if (partner != col_map.end()) {
            JoinResult res = std::make_pair(partner->second, j);
            result.push_back(res);
        }
    }

    return result;
}