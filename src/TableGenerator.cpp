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
    for (unsigned int i = 0; i < numOfLocalColumns; ++i) {
        addColumn(numOfRows, maxRandomNumberInCell, 0, table);
    }

    for (unsigned int i = 0; i < numOfRemoteColumns; ++i) {
        addColumn(numOfRows, maxRandomNumberInCell, numaNode, table);
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
    int randomRemoteNode = (((int) Random::next()) % numa_max_node()) + 1;

    for (unsigned int i = 0; i < numOfColumns; ++i) {
        addColumn(numOfRows, maxRandomNumberInCell, randomRemoteNode, table);
    }

    return table;
}

void TableGenerator::addColumn(unsigned long numOfRows, unsigned int maxRandomNumberInCell, int numaNode, Table &table) {
    auto column = std::make_shared<Column<uint32_t>>(numOfRows, numaNode);
    for (unsigned long j = 0; j < numOfRows; ++j) {
        uint32_t cellValue = (uint32_t) Random::next() % maxRandomNumberInCell;
        column.get()->data().at(j) = cellValue;
    }
    table.addColumn(column);
}
