#include <benchmark/benchmark.h>
#include <numa.h>
#include <pthread.h>

#include <iostream>
#include <limits>

#include "table.h"
#include "column.h"
#include "TableGenerator.h"

static void SetAffinity(int node) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(node, &cpuset);
    auto rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    if (rc != 0) {
        std::cerr << "Error calling pthread_setaffinity_np: " << rc << std::endl;
    }
}

static void BM_ColumnScan_1M_Rows__LocalCols__RemoteCols(benchmark::State& state) {
    auto rows = 1 * 1000 * 1000UL;
    unsigned int max_cell_value = 1000000;
    Table localTable = TableGenerator::generateTable((unsigned int) state.range(0), rows, max_cell_value, 0);
    Table remoteTable = TableGenerator::generateTable((unsigned int) state.range(1), rows, max_cell_value, numa_max_node());

    auto localCols = localTable.getColumns();
    auto remoteCols = remoteTable.getColumns();

    SetAffinity(0);

    while (state.KeepRunning())
    {
        for (ColumnPtr &col : localCols)
        {
            col->scan();
        }

        for (ColumnPtr &col : remoteCols)
        {
            col->scan();
        }
    }
}
BENCHMARK(BM_ColumnScan_1M_Rows__LocalCols__RemoteCols)
    ->RangeMultiplier(2)
    ->Ranges({
        {0, 8}, // Local columns
        {0, 8}  // Remote columns
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
