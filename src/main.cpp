#include <benchmark/benchmark.h>
#include <numa.h>
#include <pthread.h>

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
        for (ColumnPtr &col : cols)
        {
            col->scan();
        }
    }
}

static void BM_ColumnScan_1M_Rows__RemoteCols(benchmark::State& state) {
    Table table = TableGenerator::generateTable(0, num_of_remote_columns, rows, max_cell_value);;

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
        for (ColumnPtr &col : cols)
        {
            col->scan();
        }
    }
}
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

BENCHMARK_MAIN();
