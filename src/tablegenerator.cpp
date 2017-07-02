#include "tablegenerator.h"

#include <algorithm>
#include "random.h"

Table TableGenerator::generateTableOnLocalNode(
    unsigned int numOfColumns,
    unsigned long numOfRows,
    unsigned int maxRandomNumberInCell,
    int localNode
) {
    Table table;

    for (unsigned int i = 0; i < numOfColumns; ++i) {
        addColumn(numOfRows, maxRandomNumberInCell, localNode, table);
    }

    return table;
}

Table TableGenerator::generateTableOnRandomRemoteNode(
    unsigned int numOfColumns,
    unsigned long numOfRows,
    unsigned int maxRandomNumberInCell
) {
    Table table;

    int randomRemoteNode = (((unsigned int) Random::next()) % numa_max_node()) + 1;

    for (unsigned int i = 0; i < numOfColumns; ++i) {
        addColumn(numOfRows, maxRandomNumberInCell, randomRemoteNode, table);
    }

    return table;
}

Table TableGenerator::generateTableOnNodeNextToLastRemoteNode(
    unsigned int numOfColumns,
    unsigned long numOfRows,
    unsigned int maxRandomNumberInCell
) {
    Table table;

    int nodeNextTolastNode = numa_max_node() - 1;

    for (unsigned int i = 0; i < numOfColumns; ++i) {
        addColumn(numOfRows, maxRandomNumberInCell, nodeNextTolastNode, table);
    }

    return table;
}

Table TableGenerator::generateTableOnLastRemoteNode(
    unsigned int numOfColumns,
    unsigned long numOfRows,
    unsigned int maxRandomNumberInCell
) {
    Table table;

    int lastNode = numa_max_node();

    for (unsigned int i = 0; i < numOfColumns; ++i) {
        addColumn(numOfRows, maxRandomNumberInCell, lastNode, table);
    }

    return table;
}

void TableGenerator::addColumn(
    unsigned long numOfRows,
    unsigned int maxRandomNumberInCell,
    int numaNode, Table &table
) {
    auto column = std::make_shared<Column<uint32_t>>(numOfRows, numaNode);

    for (unsigned long j = 0; j < numOfRows; ++j) {
        uint32_t cellValue = (uint32_t) Random::next() % maxRandomNumberInCell;
        column.get()->data().at(j) = cellValue;
    }

    table.addColumn(column);
}

void TableGenerator::addMergeColumns(
    Table &table1,
    Table &table2,
    unsigned int matchingRows,
    unsigned int totalRows
) {
    // Determine a table's node by looking at its first column
    auto nodeTable1 = table1.column(0)->numaNode();
    auto nodeTable2 = table2.column(0)->numaNode();

    auto columnTable1 = std::make_shared<Column<uint32_t>>(totalRows, nodeTable1);
    auto columnTable2 = std::make_shared<Column<uint32_t>>(totalRows, nodeTable2);

    auto &attributeVectorTable1 = columnTable1.get()->data();
    auto &attributeVectorTable2 = columnTable2.get()->data();

    for (size_t i = 0; i < totalRows; ++i)
    {
        if (i < matchingRows) {
            attributeVectorTable1[i] = i;
        }
        attributeVectorTable2[i] = i;
    }

    // Shake well before use
    std::random_shuffle(attributeVectorTable1.begin(), attributeVectorTable1.end());
    std::random_shuffle(attributeVectorTable2.begin(), attributeVectorTable2.end());

    table1.addColumn(columnTable1);
    table2.addColumn(columnTable2);
}
