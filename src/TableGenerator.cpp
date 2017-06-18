#include "TableGenerator.h"

Table TableGenerator::generateTable(unsigned int numOfColumns, unsigned long numOfRows, unsigned int maxRandomNumberInCell, int numa_node) {
    srand(time(NULL));
    Table table;
    for (unsigned int i = 0; i < numOfColumns; ++i) {
        auto column = std::make_shared<Column<uint32_t>>(numOfRows, numa_node);
        for (unsigned long j = 0; j < numOfRows; ++j) {
            uint32_t cellValue = (uint32_t) rand() % maxRandomNumberInCell;
            column.get()->data().at(j) = cellValue;
        }
        table.addColumn(column);
    }
    return table;
}
