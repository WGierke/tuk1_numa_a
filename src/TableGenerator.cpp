#include "TableGenerator.h"
#include "random.h"

Table TableGenerator::generateTable(unsigned int numOfLocalColumns, unsigned int numOfRemoteColumns, unsigned long numOfRows, unsigned int maxRandomNumberInCell) {
    srand(time(NULL));
    Table table;
    for (unsigned int i = 0; i < numOfLocalColumns; ++i) {
        auto column = std::make_shared<Column<uint32_t>>(numOfRows, 0);
        for (unsigned long j = 0; j < numOfRows; ++j) {
            uint32_t cellValue = (uint32_t) Random::next() % maxRandomNumberInCell;
            column.get()->data().at(j) = cellValue;
        }
        table.addColumn(column);
    }
    for (unsigned int i = 0; i < numOfRemoteColumns; ++i) {
        auto column = std::make_shared<Column<uint32_t>>(numOfRows, numa_max_node());
        for (unsigned long j = 0; j < numOfRows; ++j) {
            uint32_t cellValue = (uint32_t) Random::next() % maxRandomNumberInCell;
            column.get()->data().at(j) = cellValue;
        }
        table.addColumn(column);
    }
    return table;
}
