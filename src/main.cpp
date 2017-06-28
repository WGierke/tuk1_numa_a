#include <benchmark/benchmark.h>
#include <numa.h>
#include <pthread.h>

#include <cstdlib>
#include <iostream>
#include <limits>

#include "table.h"
#include "column.h"
#include "TableGenerator.h"
#include "random.h"

static unsigned long rows = 1 * 1000 * 1000UL;
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

static void BM_ColumnScan_1M_Rows__LocalCols(benchmark::State& state) {
    Table table = TableGenerator::generateTableOnLocalNode(total_columns, rows, max_cell_value, local_node);

    std::vector<std::size_t> columnIndices;
    auto localColumns = state.range(0);

    // The table starts with 50 local columns
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

static void BM_ColumnScan_1M_Rows__RemoteCols(benchmark::State& state) {
    Table table = TableGenerator::generateTableOnRandomRemoteNode(total_columns, rows, max_cell_value);

    std::vector<std::size_t> columnIndices;
    auto remoteColumns = state.range(0);

    // The table starts with 50 local columns
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

static void BM_RowScan_1M_Rows__LocalCols(benchmark::State& state) {
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

static void BM_RowScan_1M_Rows__RemoteCols(benchmark::State& state) {
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

static void BM_Join_2k_10M_Rows__LocalTables(benchmark::State& state) {
    unsigned int colsTable1 = 10;
    unsigned long rowsTable1 = 2000;
    Table table1 = TableGenerator::generateTableOnLocalNode(colsTable1, rowsTable1, max_cell_value, local_node);

    unsigned int colsTable1 = 100;
    unsigned long rowsTable1 = 10 * rows;
    Table table2 = TableGenerator::generateTableOnLocalNode(colsTable2, rowsTable2, max_cell_value, local_node);


    TableGenerator::addMergeColumns(table1, table2, rowsTable1, rowsTable2, local_node, local_node);

    SetAffinity(local_node);

    while (state.KeepRunning())
    {
        auto res = table1.hashJoin(0, table2, 0);
    }
}

/* ************************************
    Join benchmarks
   ************************************
*/
BENCHMARK(BM_Join_2k_10M_Rows__LocalTables)
    ->Unit(benchmark::kMicrosecond);

/* ************************************
    Column scan benchmarks
   ************************************
*/
BENCHMARK(BM_ColumnScan_1M_Rows__LocalCols)
    ->RangeMultiplier(2)
    ->Ranges({
        {1, 8}, // Local columns
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_ColumnScan_1M_Rows__RemoteCols)
    ->RangeMultiplier(2)
    ->Ranges({
        {1, 8}  // Remote columns
    })
    ->Unit(benchmark::kMicrosecond);


/* ************************************
    Row scan benchmarks
   ************************************
*/
BENCHMARK(BM_RowScan_1M_Rows__LocalCols)
    ->RangeMultiplier(10)
    ->Ranges({
        {1, 100000}  // Local columns
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_RowScan_1M_Rows__RemoteCols)
    ->RangeMultiplier(10)
    ->Ranges({
        {1, 100000}  // Local columns
    })
    ->Unit(benchmark::kMicrosecond);


BENCHMARK_MAIN();
