
#ifndef PROJECT_TABLEGENERATOR_H
#define PROJECT_TABLEGENERATOR_H


#include "table.h"

class TableGenerator {

public:
    static Table generateTable(unsigned int numOfLocalColumns, unsigned int numOfRemoteColumns, unsigned long numOfRows, unsigned int maxRandomNumberInCell);

    static void addMergeColumns(Table &table1, Table &table2, unsigned int rowsTable1, unsigned int rowsTable2,
                                int numaNodeTable1, int numaNodeTable2);

};


#endif //PROJECT_TABLEGENERATOR_H
