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
