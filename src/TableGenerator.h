
#ifndef PROJECT_TABLEGENERATOR_H
#define PROJECT_TABLEGENERATOR_H


#include "table.h"

class TableGenerator {

public:
    static Table generateTable(unsigned int numOfLocalColumns, unsigned int numOfRemoteColumns, unsigned long numOfRows, unsigned int maxRandomNumberInCell);

};


#endif //PROJECT_TABLEGENERATOR_H
