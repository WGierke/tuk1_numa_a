#include "TableGenerator.h"
#include "random.h"

// TODO: remove; currently testing purpose
#include <string>
#include <iostream>


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
    int randomRemoteNode = (((int) Random::next()) % numa_max_node()) + 1;
    std::cout << "randomRemoteNode: " << randomRemoteNode;

    // TODO: remove; currently testing purpose
    std::cout << "numa_get_mems_allowed(): " << numa_get_mems_allowed();
    std::cout << "numa_all_nodes_ptr: " << numa_all_nodes_ptr();

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
