#include <benchmark/benchmark.h>
#include <numa.h>
#include <pthread.h>

#include <cstdlib>
#include <iostream>
#include <limits>

#include "table.h"
#include "column.h"
#include "tablegenerator.h"
#include "random.h"

//static unsigned long rows = 1 * 1000 * 1000UL;
static unsigned long rows_20m = 20 * 1000 * 1000UL;
static unsigned long rows_100m = 100 * 1000 * 1000UL;
static unsigned int max_cell_value = 1000000;
static unsigned int total_columns = 100;
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

    Table table1 = TableGenerator::generateTableOnLocalNode(total_columns - 1, matchingRows, max_cell_value, local_node);
    Table table2 = TableGenerator::generateTableOnLocalNode(total_columns - 1, rows, max_cell_value, local_node);

    TableGenerator::addMergeColumns(table1, table2, matchingRows, rows);

    SetAffinity(local_node);

    while (state.KeepRunning())
    {
        // Join the last two columns in the tables
        auto res = table1.hashJoin(total_columns - 1, table2, total_columns - 1);
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

    Table table1 = TableGenerator::generateTableOnLocalNode(total_columns - 1, matchingRows, max_cell_value, local_node);
    Table table2 = TableGenerator::generateTableOnLastRemoteNode(total_columns - 1, rows, max_cell_value);

    TableGenerator::addMergeColumns(table1, table2, matchingRows, rows);

    SetAffinity(local_node);

    while (state.KeepRunning())
    {
        // Join the last two columns in the tables
        auto res = table1.hashJoin(total_columns - 1, table2, total_columns - 1);
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

    Table table1 = TableGenerator::generateTableOnLastRemoteNode(total_columns - 1, matchingRows, max_cell_value);
    Table table2 = TableGenerator::generateTableOnLastRemoteNode(total_columns - 1, rows, max_cell_value);

    TableGenerator::addMergeColumns(table1, table2, matchingRows, rows);

    SetAffinity(local_node);

    while (state.KeepRunning())
    {
        // Join the last two columns in the tables
        auto res = table1.hashJoin(total_columns - 1, table2, total_columns - 1);
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

    Table table1 = TableGenerator::generateTableOnNodeNextToLastRemoteNode(total_columns - 1, matchingRows, max_cell_value);
    Table table2 = TableGenerator::generateTableOnLastRemoteNode(total_columns - 1, rows, max_cell_value);

    TableGenerator::addMergeColumns(table1, table2, matchingRows, rows);

    SetAffinity(local_node);

    while (state.KeepRunning())
    {
        // Join the last two columns in the tables
        auto res = table1.hashJoin(total_columns - 1, table2, total_columns - 1);
    }
}

static void BM_Join_20M_Rows__DifferentRemoteTables(benchmark::State& state) {
    BM_Join__DifferentRemoteTables(state, rows_20m);
}

static void BM_Join_100M_Rows__DifferentRemoteTables(benchmark::State& state) {
    BM_Join__DifferentRemoteTables(state, rows_100m);
}

/* ************************************
    Join benchmarks
   ************************************
*/
BENCHMARK(BM_Join_20M_Rows__LocalTables)
    ->RangeMultiplier(10)
    ->Ranges({
        {2000, 2000000},
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
        {2000, 2000000},
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_Join_100M_Rows__SameRemoteTables)
    ->RangeMultiplier(10)
    ->Ranges({
        {2000, 2000000},
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_Join_20M_Rows__DifferentRemoteTables)
    ->RangeMultiplier(10)
    ->Ranges({
        {2000, 2000000},
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
