#include "tablegenerator.h"

#include <algorithm>
#include "random.h"

Table TableGenerator::generateTableOnLocalNode(
    unsigned int numOfColumns,
    unsigned long numOfRows,
    unsigned int maxRandomNumberInCell,
    int localNode
) {
    Table table(numOfRows);

    for (unsigned int i = 0; i < numOfColumns; ++i) {
        addColumn(maxRandomNumberInCell, localNode, table);
    }

    return table;
}

Table TableGenerator::generateTableOnRandomRemoteNode(
    unsigned int numOfColumns,
    unsigned long numOfRows,
    unsigned int maxRandomNumberInCell
) {
    Table table(numOfRows);

    int randomRemoteNode = (((unsigned int) Random::next()) % numa_max_node()) + 1;

    for (unsigned int i = 0; i < numOfColumns; ++i) {
        addColumn(maxRandomNumberInCell, randomRemoteNode, table);
    }

    return table;
}

Table TableGenerator::generateTableOnNodeNextToLastRemoteNode(
    unsigned int numOfColumns,
    unsigned long numOfRows,
    unsigned int maxRandomNumberInCell
) {
    Table table(numOfRows);

    int nodeNextTolastNode = numa_max_node() - 1;

    for (unsigned int i = 0; i < numOfColumns; ++i) {
        addColumn(maxRandomNumberInCell, nodeNextTolastNode, table);
    }

    return table;
}

Table TableGenerator::generateTableOnLastRemoteNode(
    unsigned int numOfColumns,
    unsigned long numOfRows,
    unsigned int maxRandomNumberInCell
) {
    Table table(numOfRows);

    int lastNode = numa_max_node();

    for (unsigned int i = 0; i < numOfColumns; ++i) {
        addColumn(maxRandomNumberInCell, lastNode, table);
    }

    return table;
}

void TableGenerator::addColumn(
    unsigned int maxRandomNumberInCell,
    int numaNode, Table &table
) {
    auto column = std::make_shared<Column<uint32_t>>(table.rows(), numaNode);

    for (unsigned long j = 0; j < table.rows(); ++j) {
        uint32_t cellValue = (uint32_t) Random::next() % maxRandomNumberInCell;
        column.get()->data().at(j) = cellValue;
    }

    table.addColumn(column);
}

void TableGenerator::addMergeColumns(
    Table &table1,
    Table &table2,
    unsigned int matchingRows
) {
    // Determine a table's node by looking at its first column
    auto nodeTable1 = table1.column(0)->numaNode();
    auto nodeTable2 = table2.column(0)->numaNode();

    auto columnTable1 = std::make_shared<Column<uint32_t>>(table1.rows(), nodeTable1);
    auto columnTable2 = std::make_shared<Column<uint32_t>>(table2.rows(), nodeTable2);

    auto &attributeVectorTable1 = columnTable1.get()->data();
    auto &attributeVectorTable2 = columnTable2.get()->data();

    size_t currentRowTable1 = 0;
    size_t currentRowTable2 = 0;
    for (size_t i = 0;; ++i)
    {
        auto generateMatchingRows = i < matchingRows;
        auto generateRowOnlyForTable1 = i >= matchingRows && i < table1.rows();
        auto generateRowOnlyForTable2 = i >= table1.rows() && i - table1.rows() < table2.rows() - matchingRows;

        if (generateMatchingRows || generateRowOnlyForTable1)
        {
            attributeVectorTable1[currentRowTable1++] = i;
        }

        if (generateMatchingRows || generateRowOnlyForTable2)
        {
            attributeVectorTable2[currentRowTable2++] = i;
        }

        if (!generateMatchingRows && !generateRowOnlyForTable1 && !generateRowOnlyForTable2)
        {
            break;
        }
    }

    // Shake well before use
    std::random_shuffle(attributeVectorTable1.begin(), attributeVectorTable1.end());
    std::random_shuffle(attributeVectorTable2.begin(), attributeVectorTable2.end());

    table1.addColumn(columnTable1);
    table2.addColumn(columnTable2);
}
