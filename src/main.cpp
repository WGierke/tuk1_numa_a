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
    SetAffinity(local_node);

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
    SetAffinity(local_node);

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
    SetAffinity(local_node);

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
    SetAffinity(local_node);

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

    SetAffinity(local_node);

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

    SetAffinity(local_node);

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

    SetAffinity(local_node);

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

    SetAffinity(local_node);

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

static void BM_Task3(benchmark::State& state) {

    auto benchmarkCase = state.range(0); // 0 = Local-Local, 1 = Local-Remote
    auto rows = state.range(1) * 1000 * 1000;

    // FYI: 2 Tables * 100M Rows * 200 Cols * 4 Byte = 160GB
    // Each node on gaza has ~128 GB memory, so we have a problem in the local-local case

    SetAffinity(local_node);

    auto tableA = TableGenerator::generateTableOnLocalNode(20, rows, max_cell_value, local_node);
    auto tableB = benchmarkCase == 0
        ? TableGenerator::generateTableOnLocalNode(20, rows, max_cell_value, local_node)
        : TableGenerator::generateTableOnLastRemoteNode(20, rows, max_cell_value);

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
        // TODO -- Different approaches to test:
        // Filter at the beginning or after the join
        // Materialize at the beginning or at the end

        // Maybe 'materializing' the row indices (as below) is already too much?
        // Should we rather combine different expressions into one pipeline without any
        // intermediate results (i.e. vector<something>)?

        // WHERE A.col10 = 42
        // Selectivity 1%
        auto rowsTableAFiltered = tableA.where(10, [](const uint32_t &x) { return x == 42; });

        // Because of fewer rows and a smaller column cardinality we perform the hash join
        // by generating the hash map for A's values
        auto rowsTableATableBJoined = tableA.hashJoinRows(9, rowsTableAFiltered, tableB, 1);

        // Materialize
        std::vector<std::vector<uint32_t>> result(rowsTableATableBJoined.size(), std::vector<uint32_t>(8));
        for (std::size_t i = 0; i < rowsTableATableBJoined.size(); ++i)
        {
            for (std::size_t j = 1; j <= 8; ++j)
            {
                result[i][j] = tableB.column(j)->valueAt(rowsTableATableBJoined[i].second);
            }
        }
    }
}

/* ************************************
    Task 3 benchmarks
   ************************************
*/

BENCHMARK(BM_Task3)
    ->Args({0, 1})
    // local-local not possible, see above
    // ->Args({0, 100})
    ->Args({1, 1})
    ->Args({1, 100})
    ->Unit(benchmark::kMillisecond);

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
