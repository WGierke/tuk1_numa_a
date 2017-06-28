#include "TableGenerator.h"
#include "random.h"

Table TableGenerator::generateTableOnLocalNode(
    unsigned int numOfColumns,
    unsigned long numOfRows,
    unsigned int maxRandomNumberInCell,
    int localNode
) {
    srand(time(NULL));
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
    srand(time(NULL));
    Table table;
    int randomRemoteNode = (((unsigned int) Random::next()) % numa_max_node()) + 1;

    for (unsigned int i = 0; i < numOfColumns; ++i) {
        addColumn(numOfRows, maxRandomNumberInCell, randomRemoteNode, table);
    }

    return table;
}


Table TableGenerator::generateTableOnLastRemoteNode(
    unsigned int numOfColumns,
    unsigned long numOfRows,
    unsigned int maxRandomNumberInCell
) {
    srand(time(NULL));
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
    unsigned int rowsTable1,
    unsigned int rowsTable2,
    int nodeTable1,
    int nodeTable2
) {
    auto columnTable1 = std::make_shared<Column<uint32_t>>(rowsTable1, nodeTable1);

    for (unsigned int i = 0; i < rowsTable1; ++i) {
        columnTable1.get()->data().at(i) = i + 1;
    }

    table1.addColumn(columnTable1);

    auto columnTable2 = std::make_shared<Column<uint32_t>>(rowsTable2, nodeTable2);

    for (unsigned int i = 0; i < rowsTable2/2; ++i) {
        columnTable2.get()->data().at(i) = i + 1;
    }

    for (unsigned int i = rowsTable2/2, j = rowsTable1 + 1; i < rowsTable2; ++i, ++j) {
        columnTable2.get()->data().at(i) = j;
    }

    table2.addColumn(columnTable2);
}
