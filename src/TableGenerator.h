
#ifndef PROJECT_TABLEGENERATOR_H
#define PROJECT_TABLEGENERATOR_H


#include "table.h"

class TableGenerator {

public:
    static Table generateTable(unsigned int numOfColumns, unsigned long numOfRows, unsigned int maxRandomNumberInCell, int numa_node);

};


#endif //PROJECT_TABLEGENERATOR_H
