#include "TableGenerator.h"

Table TableGenerator::generateTable(unsigned int numOfLocalColumns, unsigned int numOfRemoteColumns,
                                    unsigned long numOfRows, unsigned int maxRandomNumberInCell, int numaNode) {
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

Table TableGenerator::generateTable(unsigned int numOfLocalColumns, unsigned int numOfRemoteColumns,
                                    unsigned long numOfRows, unsigned int maxRandomNumberInCell) {
    srand(time(NULL));
    Table table;
    for (unsigned int i = 0; i < numOfLocalColumns; ++i) {
        addColumn(numOfRows, maxRandomNumberInCell, 0, table);
    }
    for (unsigned int i = 0; i < numOfRemoteColumns; ++i) {
        addColumn(numOfRows, maxRandomNumberInCell, (rand() % (numa_max_node() + 1)), table);
    }
    return table;
}

void TableGenerator::addColumn(unsigned long numOfRows, unsigned int maxRandomNumberInCell, int numaNode, Table &table) {
    auto column = std::make_shared<Column<uint32_t>>(numOfRows, numaNode);
    for (unsigned long j = 0; j < numOfRows; ++j) {
        uint32_t cellValue = (uint32_t) rand() % maxRandomNumberInCell;
        column.get()->data().at(j) = cellValue;
    }
    table.addColumn(column);
}
