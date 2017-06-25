#include <benchmark/benchmark.h>
#include <numa.h>
#include <pthread.h>

#include <cstdlib> 
#include <iostream>
#include <limits>

#include "table.h"
#include "column.h"
#include "TableGenerator.h"

static auto rows = 1 * 1000 * 1000UL;
static unsigned int max_cell_value = 1000000;
static unsigned int num_of_local_columns = 50;
static unsigned int num_of_remote_columns = 50;

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
    std::vector<size_t> indices;
    for (uint i = 0; i < num_indices; ++i) 
    {
        // Random index in range 0 to max_index (both inclusive)
        indices.push_back(rand() % (max_index + 1));
    }

    return indices;
}

static void BM_ColumnScan_1M_Rows__LocalCols(benchmark::State& state) {
    Table table = TableGenerator::generateTable(num_of_local_columns, 0, rows, max_cell_value);

    std::vector<std::size_t> columnIndices;
    auto localColumns = state.range(0);

    // The table starts with 50 local columns
    for (auto i = 0; i < localColumns; ++i) {
         columnIndices.push_back(i);
    }

    auto cols = table.getColumns(columnIndices);
    SetAffinity(0);
    while (state.KeepRunning())
    {
        for (auto &col : cols)
        {
            col->scan();
        }
    }
}

static void BM_ColumnScan_1M_Rows__RemoteCols(benchmark::State& state) {
    Table table = TableGenerator::generateTable(0, num_of_remote_columns, rows, max_cell_value);

    std::vector<std::size_t> columnIndices;
    auto remoteColumns = state.range(0);

    // The table starts with 50 local columns
    for (auto i = 0; i < remoteColumns; ++i) {
         columnIndices.push_back(i);
    }

    auto cols = table.getColumns(columnIndices);
    SetAffinity(0);
    while (state.KeepRunning())
    {
        for (auto &col : cols)
        {
            col->scan();
        }
    }
}


static void BM_RowScan_1M_Rows__LocalCols(benchmark::State& state) {
    Table table = TableGenerator::generateTable(num_of_local_columns, 0, rows, max_cell_value);

    std::vector<std::size_t> columnIndices;
    auto numRows = state.range(0);
    SetAffinity(0);

    auto rowIndices = randomIndices(numRows, rows - 1);

    std::vector<std::vector<uint32_t>> results;
    while (state.KeepRunning())
    {
        results = table.scanRows(rowIndices);
    }
}

static void BM_RowScan_1M_Rows__RemoteCols(benchmark::State& state) {
    Table table = TableGenerator::generateTable(0, num_of_remote_columns, rows, max_cell_value);

    std::vector<std::size_t> columnIndices;
    auto numRows = state.range(0);
    SetAffinity(0);

    auto rowIndices = randomIndices(numRows, rows - 1);

    std::vector<std::vector<uint32_t>> results;
    while (state.KeepRunning())
    {
        results = table.scanRows(rowIndices);
    }
}

static void BM_Join_1M_Rows__LocalTables(benchmark::State& state) {
    Table table = TableGenerator::generateTable(2*num_of_local_columns, 0, rows, max_cell_value);
    Table other = TableGenerator::generateTable(2*num_of_local_columns, 0, rows, max_cell_value);

    SetAffinity(0);


    while (state.KeepRunning()) 
    {
        auto res = table.hashJoin(0, other, 0);
    }

}
/* ************************************
    Join benchmarks
   ************************************ 
*/
BENCHMARK(BM_Join_1M_Rows__LocalTables)
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
