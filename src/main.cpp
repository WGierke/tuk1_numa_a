#include <benchmark/benchmark.h>
#include <numa.h>
#include <pthread.h>

#include <cstdlib>
#include <iostream>
#include <limits>
#include <algorithm>

#include "table.h"
#include "column.h"
#include "tablegenerator.h"
#include "random.h"

//static unsigned long rows = 1 * 1000 * 1000UL;
static unsigned long rows_20m = 20 * 1000 * 1000UL;
static unsigned long rows_100m = 100 * 1000 * 1000UL;
static unsigned int max_cell_value = 1000000;
static unsigned int total_columns = 100;
static unsigned int total_columns_for_join_benchmarks = 10;
static int local_node = 0;
static int local_core = 0;

static void SetAffinity(int node) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(node, &cpuset);
    auto rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    if (rc != 0) {
        std::cerr << "Error calling pthread_setaffinity_np: " << rc << std::endl;
    }
}

std::vector<size_t> randomIndices(size_t num_indices, size_t max_index) {
    std::vector<size_t> indices(num_indices);
    for (uint i = 0; i < num_indices; ++i)
    {
        // Random index in range 0 to max_index (both inclusive)
        indices[i] = Random::next() % (max_index + 1);
    }

    return indices;
}

static void BM_ColumnScan_LocalCols(benchmark::State &state, unsigned long rows) {
    Table table = TableGenerator::generateTableOnLocalNode(total_columns, rows, max_cell_value, local_node);

    std::vector<std::size_t> columnIndices;
    auto localColumns = state.range(0);

    for (auto i = 0; i < localColumns; ++i) {
         columnIndices.push_back(i);
    }

    auto cols = table.getColumns(columnIndices);
    SetAffinity(local_core);

    while (state.KeepRunning())
    {
        for (auto &col : cols)
        {
            col->scan();
        }
    }
}

static void BM_ColumnScan_20M_Rows__LocalCols(benchmark::State& state) {
    BM_ColumnScan_LocalCols(state, rows_20m);
}

static void BM_ColumnScan_100M_Rows__LocalCols(benchmark::State& state) {
    BM_ColumnScan_LocalCols(state, rows_100m);
}

static void BM_ColumnScan_RemoteCols(benchmark::State &state, unsigned long rows) {
    Table table = TableGenerator::generateTableOnRandomRemoteNode(total_columns, rows, max_cell_value);

    std::vector<std::size_t> columnIndices;
    auto remoteColumns = state.range(0);

    for (auto i = 0; i < remoteColumns; ++i) {
        columnIndices.push_back(i);
    }

    auto cols = table.getColumns(columnIndices);
    SetAffinity(local_core);

    while (state.KeepRunning())
    {
        for (auto &col : cols)
        {
            col->scan();
        }
    }
}

static void BM_ColumnScan_20M_Rows__RemoteCols(benchmark::State& state) {
    BM_ColumnScan_RemoteCols(state, rows_20m);
}

static void BM_ColumnScan_100M_Rows__RemoteCols(benchmark::State& state) {
    BM_ColumnScan_RemoteCols(state, rows_100m);
}

static void BM_RowScan_LocalCols(benchmark::State &state, unsigned long rows) {
    Table table = TableGenerator::generateTableOnLocalNode(total_columns, rows, max_cell_value, local_node);

    std::vector<std::size_t> columnIndices;
    auto numRows = state.range(0);
    SetAffinity(local_core);

    std::vector<std::vector<uint32_t>> results;

    while (state.KeepRunning())
    {
        state.PauseTiming();
        auto rowIndices = randomIndices(numRows, rows - 1);
        state.ResumeTiming();
        results = table.scanRows(rowIndices);
    }
}

static void BM_RowScan_20M_Rows__LocalCols(benchmark::State& state) {
    BM_RowScan_LocalCols(state, rows_20m);
}

static void BM_RowScan_100M_Rows__LocalCols(benchmark::State& state) {
    BM_RowScan_LocalCols(state, rows_100m);
}

static void BM_RowScan_RemoteCols(benchmark::State &state, unsigned long rows) {
    Table table = TableGenerator::generateTableOnRandomRemoteNode(total_columns, rows, max_cell_value);

    std::vector<std::size_t> columnIndices;
    auto numRows = state.range(0);
    SetAffinity(local_core);

    std::vector<std::vector<uint32_t>> results;
    while (state.KeepRunning())
    {
        state.PauseTiming();
        auto rowIndices = randomIndices(numRows, rows - 1);
        state.ResumeTiming();
        results = table.scanRows(rowIndices);
    }
}

static void BM_RowScan_20M_Rows__RemoteCols(benchmark::State& state) {
    BM_RowScan_RemoteCols(state, rows_20m);
}

static void BM_RowScan_100M_Rows__RemoteCols(benchmark::State& state) {
    BM_RowScan_RemoteCols(state, rows_100m);
}

static void BM_Join__LocalTables(benchmark::State& state, unsigned long rows) {
    auto matchingRows = state.range(0);
    auto table1Rows = matchingRows * 2;  // = number of distinct dictionary entries
    auto table2Rows = rows;              // = number of rows to probe

    Table table1 = TableGenerator::generateTableOnLocalNode(total_columns_for_join_benchmarks - 1, table1Rows, max_cell_value, local_node);
    Table table2 = TableGenerator::generateTableOnLocalNode(total_columns_for_join_benchmarks - 1, table2Rows, max_cell_value, local_node);

    TableGenerator::addMergeColumns(table1, table2, matchingRows);

    SetAffinity(local_core);

    while (state.KeepRunning())
    {
        // Join the last two columns in the tables
        auto res = table1.hashJoin(total_columns_for_join_benchmarks - 1, table2, total_columns_for_join_benchmarks - 1);
    }
}

static void BM_Join_20M_Rows__LocalTables(benchmark::State& state) {
    BM_Join__LocalTables(state, rows_20m);
}

static void BM_Join_100M_Rows__LocalTables(benchmark::State& state) {
    BM_Join__LocalTables(state, rows_100m);
}

static void BM_Join__LocalTable_RemoteTable(benchmark::State& state, unsigned long rows) {
    auto matchingRows = state.range(0);
    auto table1Rows = matchingRows * 2;  // = number of distinct dictionary entries
    auto table2Rows = rows;              // = number of rows to probe

    Table table1 = TableGenerator::generateTableOnLocalNode(total_columns_for_join_benchmarks - 1, table1Rows, max_cell_value, local_node);
    Table table2 = TableGenerator::generateTableOnLastRemoteNode(total_columns_for_join_benchmarks - 1, table2Rows, max_cell_value);

    TableGenerator::addMergeColumns(table1, table2, matchingRows);

    SetAffinity(local_core);

    while (state.KeepRunning())
    {
        // Join the last two columns in the tables
        auto res = table1.hashJoin(total_columns_for_join_benchmarks - 1, table2, total_columns_for_join_benchmarks - 1);
    }
}

static void BM_Join_20M_Rows__LocalTable_RemoteTable(benchmark::State& state) {
    BM_Join__LocalTable_RemoteTable(state, rows_20m);
}

static void BM_Join_100M_Rows__LocalTable_RemoteTable(benchmark::State& state) {
    BM_Join__LocalTable_RemoteTable(state, rows_100m);
}

static void BM_Join__SameRemoteTables(benchmark::State& state, unsigned long rows) {
    auto matchingRows = state.range(0);
    auto table1Rows = matchingRows * 2;  // = number of distinct dictionary entries
    auto table2Rows = rows;              // = number of rows to probe

    Table table1 = TableGenerator::generateTableOnLastRemoteNode(total_columns_for_join_benchmarks - 1, table1Rows, max_cell_value);
    Table table2 = TableGenerator::generateTableOnLastRemoteNode(total_columns_for_join_benchmarks - 1, table2Rows, max_cell_value);

    TableGenerator::addMergeColumns(table1, table2, matchingRows);

    SetAffinity(local_core);

    while (state.KeepRunning())
    {
        // Join the last two columns in the tables
        auto res = table1.hashJoin(total_columns_for_join_benchmarks - 1, table2, total_columns_for_join_benchmarks - 1);
    }
}

static void BM_Join_20M_Rows__SameRemoteTables(benchmark::State& state) {
    BM_Join__SameRemoteTables(state, rows_20m);
}

static void BM_Join_100M_Rows__SameRemoteTables(benchmark::State& state) {
    BM_Join__SameRemoteTables(state, rows_100m);
}

static void BM_Join__DifferentRemoteTables(benchmark::State& state, unsigned long rows) {
    auto matchingRows = state.range(0);
    auto table1Rows = matchingRows * 2;  // = number of distinct dictionary entries
    auto table2Rows = rows;              // = number of rows to probe

    Table table1 = TableGenerator::generateTableOnNodeNextToLastRemoteNode(total_columns_for_join_benchmarks - 1, table1Rows, max_cell_value);
    Table table2 = TableGenerator::generateTableOnLastRemoteNode(total_columns_for_join_benchmarks - 1, table2Rows, max_cell_value);

    TableGenerator::addMergeColumns(table1, table2, matchingRows);

    SetAffinity(local_core);

    while (state.KeepRunning())
    {
        // Join the last two columns in the tables
        auto res = table1.hashJoin(total_columns_for_join_benchmarks - 1, table2, total_columns_for_join_benchmarks - 1);
    }
}

static void BM_Join_20M_Rows__DifferentRemoteTables(benchmark::State& state) {
    BM_Join__DifferentRemoteTables(state, rows_20m);
}

static void BM_Join_100M_Rows__DifferentRemoteTables(benchmark::State& state) {
    BM_Join__DifferentRemoteTables(state, rows_100m);
}

static void performTask3QueryWithEarlyMaterialization(Table &tableA, Table &tableB, bool earlySelection)
{
    std::vector<size_t> everything = { std::numeric_limits<size_t>::max() };
    std::vector<size_t> nothing = {};

    Table tableATableBJoinedAndFiltered(0);
    std::vector<size_t> projectColumns = { 1, 2, 3, 4, 5, 6, 7, 8 };

    if (earlySelection)
    {
        // WHERE A.col10 = 42
        // Selectivity 1%
        std::vector<size_t> projectColumnsA = { 9 };
        auto tableAFiltered = tableA.where(10, [&](const uint32_t &x, const Table::materializer &m, size_t row) {
            if (x == 42)
                m(projectColumnsA, row);
        });

        // Because of fewer rows and a smaller column cardinality we perform the hash join
        // by generating the hash map for A's values
        // Column 0 is now Table A's column 9
        tableATableBJoinedAndFiltered = tableAFiltered.hashJoinRows(0, nothing, tableB, 1, [&](size_t rowA, size_t rowB, const Table::joinMaterializer &m){
            m(nothing, projectColumns, rowA, rowB);
        });
    }
    else
    {
        std::vector<size_t> projectColumnsA = { 10 };
        auto tableATableBJoined = tableA.hashJoinRows(9, nothing, tableB, 1, [&](size_t rowA, size_t rowB, const Table::joinMaterializer &m){
            m(projectColumnsA, projectColumns, rowA, rowB);
        });

        // Column 0 is now Table A's column 10
        tableATableBJoinedAndFiltered = tableATableBJoined.where(0, [&](const uint32_t &x, const Table::materializer &m, size_t row) {
            if (x == 42)
                m(projectColumns /* still valid in the combined table */, row);
        });
    }

    // Materialize row-oriented, thereby copying to local node
    std::vector<std::vector<uint32_t>> result(tableATableBJoinedAndFiltered.rows(), std::vector<uint32_t>(projectColumns.size()));
    for (std::size_t i = 0; i < tableATableBJoinedAndFiltered.rows(); ++i)
    {
        for (std::size_t j = 0; j < projectColumns.size(); ++j)
        {
            result[i][j] = tableATableBJoinedAndFiltered.column(j)->valueAt(i);
        }
    }
}

static void performTask3QueryWithLateMaterialization(Table &tableA, Table &tableB, bool earlySelection)
{
    // Maybe 'materializing' the row indices (as below) is already too much?
    // Should we rather combine different expressions into one pipeline without any
    // intermediate results (i.e. vector<something>)?

    std::vector<size_t> everything = { std::numeric_limits<size_t>::max() };
    std::vector<size_t> nothing = {};

    std::vector<std::pair<size_t, size_t>> rowsTableATableBJoinedAndFiltered;
    if (earlySelection)
    {
        // WHERE A.col10 = 42
        // Selectivity 1%
        std::vector<std::size_t> rowsTableAFiltered;
        tableA.where(10, [&](const uint32_t &x, const Table::materializer&, size_t row) {
            if (x == 42)
                rowsTableAFiltered.emplace_back(row);
        });
        // Because of fewer rows and a smaller column cardinality we perform the hash join
        // by generating the hash map for A's values
        tableA.hashJoinRows(9, rowsTableAFiltered, tableB, 1, [&](size_t rowA, size_t rowB, const Table::joinMaterializer &m){
            rowsTableATableBJoinedAndFiltered.emplace_back(rowA, rowB);
        });
    }
    else
    {
        std::vector<std::pair<size_t, size_t>> rowsTableATableBJoined;
        tableA.hashJoinRows(9, nothing, tableB, 1, [&](size_t rowA, size_t rowB, const Table::joinMaterializer &m){
            rowsTableATableBJoined.emplace_back(rowA, rowB);
        });

        // Filter (had to be reimplemented here... :/ )
        auto &filterColumn = *tableA.column(10);
        for (auto &joined : rowsTableATableBJoined)
        {
            if (filterColumn.valueAt(joined.first) == 42)
                rowsTableATableBJoinedAndFiltered.emplace_back(joined);
        }
    }

    // Materialize
    std::vector<std::vector<uint32_t>> result(rowsTableATableBJoinedAndFiltered.size(), std::vector<uint32_t>(8));
    for (std::size_t i = 0; i < rowsTableATableBJoinedAndFiltered.size(); ++i)
    {
        for (std::size_t j = 1; j <= 8; ++j)
        {
            result[i][j] = tableB.column(j)->valueAt(rowsTableATableBJoinedAndFiltered[i].second);
        }
    }
}

static void BM_Task3(benchmark::State& state) {

    auto benchmarkCase = state.range(0); // 0 = Local-Local, 1 = Local-Remote
    auto earlyMaterialization = state.range(1);
    auto earlySelection = state.range(2);
    auto rows = state.range(3) * 1000 * 1000;
    auto columns = 100;

    // FYI: 2 Tables * 100M Rows * 100 Cols * 4 Byte = 80GB

    SetAffinity(local_core);

    auto tableA = TableGenerator::generateTableOnLocalNode(columns, rows, max_cell_value, local_node);
    auto tableB = benchmarkCase == 0
        ? TableGenerator::generateTableOnLocalNode(columns, rows, max_cell_value, local_node)
        : TableGenerator::generateTableOnLastRemoteNode(columns, rows, max_cell_value);

    // Adjust column contents

    // Table A column 10 is supposed to include values between 0 and 99 (uniform distributed)
    size_t v = 0;
    for (auto &attribute : tableA.column(10)->data()){
        attribute = (v++) % 100;
    }
    std::random_shuffle(tableA.column(10)->data().begin(), tableA.column(10)->data().end());

    // Column 9 in Table A has values in the range of (1 – 10,000)
    for (auto &attribute : tableA.column(9)->data()){
        attribute = ((uint32_t) Random::next() % 10000) + 1;
    }

    // Column 1 in Table B is unique with values from 1 to length of table
    v = 0;
    for (auto &attribute : tableB.column(1)->data()){
        attribute = v++;
    }
    std::random_shuffle(tableB.column(1)->data().begin(), tableB.column(1)->data().end());

    while (state.KeepRunning())
    {
        if (earlyMaterialization) {
            performTask3QueryWithEarlyMaterialization(tableA, tableB, earlySelection);
        } else if (!earlyMaterialization) {
            performTask3QueryWithLateMaterialization(tableA, tableB, earlySelection);
        }
    }
}

/* ************************************
    Task 3 benchmarks
   ************************************
*/

BENCHMARK(BM_Task3)
    ->RangeMultiplier(100)
    ->Ranges({{0, 1},  // local-local, local-remote
              {0, 1},  // early, late materialization
              {0, 1},  // early, late selection
              {1, 100}}) // million rows
    ->Unit(benchmark::kMicrosecond);

/* ************************************
    Join benchmarks
   ************************************
*/
BENCHMARK(BM_Join_20M_Rows__LocalTables)
    ->RangeMultiplier(10)
    ->Ranges({
        {1000, 1000000},
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_Join_100M_Rows__LocalTables)
    ->RangeMultiplier(10)
    ->Ranges({
        {1000, 1000000},
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_Join_20M_Rows__LocalTable_RemoteTable)
    ->RangeMultiplier(10)
    ->Ranges({
        {1000, 1000000},
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_Join_100M_Rows__LocalTable_RemoteTable)
    ->RangeMultiplier(10)
    ->Ranges({
        {1000, 1000000},
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_Join_20M_Rows__SameRemoteTables)
    ->RangeMultiplier(10)
    ->Ranges({
        {1000, 1000000},
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_Join_100M_Rows__SameRemoteTables)
    ->RangeMultiplier(10)
    ->Ranges({
        {1000, 1000000},
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_Join_20M_Rows__DifferentRemoteTables)
    ->RangeMultiplier(10)
    ->Ranges({
        {1000, 1000000},
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_Join_100M_Rows__DifferentRemoteTables)
    ->RangeMultiplier(10)
    ->Ranges({
        {1000, 1000000},
    })
    ->Unit(benchmark::kMicrosecond);

/* ************************************
    Column scan benchmarks
   ************************************
*/
BENCHMARK(BM_ColumnScan_20M_Rows__LocalCols)
    ->RangeMultiplier(2)
    ->Ranges({
        {1, 8}, // Local columns
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_ColumnScan_100M_Rows__LocalCols)
    ->RangeMultiplier(2)
    ->Ranges({
        {1, 8}, // Local columns
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_ColumnScan_20M_Rows__RemoteCols)
    ->RangeMultiplier(2)
    ->Ranges({
        {1, 8}  // Remote columns
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_ColumnScan_100M_Rows__RemoteCols)
    ->RangeMultiplier(2)
    ->Ranges({
        {1, 8}  // Remote columns
    })
    ->Unit(benchmark::kMicrosecond);


/* ************************************
    Row scan benchmarks
   ************************************
*/
BENCHMARK(BM_RowScan_20M_Rows__LocalCols)
    ->RangeMultiplier(10)
    ->Ranges({
        {1, 100000}  // Local columns
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_RowScan_100M_Rows__LocalCols)
    ->RangeMultiplier(10)
    ->Ranges({
        {1, 100000}  // Local columns
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_RowScan_20M_Rows__RemoteCols)
    ->RangeMultiplier(10)
    ->Ranges({
        {1, 100000}  // Local columns
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_RowScan_100M_Rows__RemoteCols)
    ->RangeMultiplier(10)
    ->Ranges({
        {1, 100000}  // Local columns
    })
    ->Unit(benchmark::kMicrosecond);


BENCHMARK_MAIN();
