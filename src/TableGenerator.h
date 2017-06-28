
#ifndef PROJECT_TABLEGENERATOR_H
#define PROJECT_TABLEGENERATOR_H


#include "table.h"

class TableGenerator {

public:
    static Table generateTableOnLocalNode(
        unsigned int numOfColumns,
        unsigned long numOfRows,
        unsigned int maxRandomNumberInCell,
        int localNode
    );

    static Table generateTableOnRandomRemoteNode(
        unsigned int numOfColumns,
        unsigned long numOfRows,
        unsigned int maxRandomNumberInCell
    );

    static Table generateTableOnPenultimatetRemoteNode(
        unsigned int numOfColumns,
        unsigned long numOfRows,
        unsigned int maxRandomNumberInCell
    );

    static Table generateTableOnLastRemoteNode(
        unsigned int numOfColumns,
        unsigned long numOfRows,
        unsigned int maxRandomNumberInCell
    );

    static void addColumn(
        unsigned long numOfRows,
        unsigned int maxRandomNumberInCell,
        int numaNode,
        Table &table
    );

    static void addMergeColumns(
        Table &table1,
        Table &table2,
        unsigned int rowsTable1,
        unsigned int rowsTable2,
        int nodeTable1,
        int nodeTable2
    );
};


#endif //PROJECT_TABLEGENERATOR_H
