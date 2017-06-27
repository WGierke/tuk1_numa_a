#include "TableGenerator.h"

Table TableGenerator::generateTable(unsigned int numOfLocalColumns, unsigned int numOfRemoteColumns, unsigned long numOfRows, unsigned int maxRandomNumberInCell) {
    srand(time(NULL));
    Table table;
    for (unsigned int i = 0; i < numOfLocalColumns; ++i) {
        auto column = std::make_shared<Column<uint32_t>>(numOfRows, 0);
        for (unsigned long j = 0; j < numOfRows; ++j) {
            uint32_t cellValue = (uint32_t) rand() % maxRandomNumberInCell;
            column.get()->data().at(j) = cellValue;
        }
        table.addColumn(column);
    }
    for (unsigned int i = 0; i < numOfRemoteColumns; ++i) {
        auto column = std::make_shared<Column<uint32_t>>(numOfRows, numa_max_node());
        for (unsigned long j = 0; j < numOfRows; ++j) {
            uint32_t cellValue = (uint32_t) rand() % maxRandomNumberInCell;
            column.get()->data().at(j) = cellValue;
        }
        table.addColumn(column);
    }
    return table;
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
