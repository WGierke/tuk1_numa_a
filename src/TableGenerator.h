
#ifndef PROJECT_TABLEGENERATOR_H
#define PROJECT_TABLEGENERATOR_H


#include "table.h"

class TableGenerator {

public:
    static Table generateTable(unsigned int numOfLocalColumns, unsigned int numOfRemoteColumns, unsigned long numOfRows,
                               unsigned int maxRandomNumberInCell, int numaNode);

    static Table generateTable(unsigned int numOfLocalColumns, unsigned int numOfRemoteColumns, unsigned long numOfRows,
                               unsigned int maxRandomNumberInCell);

    static void addColumn(unsigned long numOfRows, unsigned int maxRandomNumberInCell, int numaNode, Table &table);
};


#endif //PROJECT_TABLEGENERATOR_H
