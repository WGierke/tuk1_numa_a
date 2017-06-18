#include <benchmark/benchmark.h>
#include <numa.h>
#include <pthread.h>

#include <iostream>
#include <limits>

#include "table.h"
#include "column.h"

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
    Table table;

    std::vector<std::size_t> columnIndices;

    auto localColumns = state.range(0);

    auto rows = 1 * 1000 * 1000UL;

    for (auto i = 0; i < localColumns; ++i)
    {
        auto column = std::make_shared<Column<uint32_t>>(rows, 0);
        auto columnIndex = table.addColumn(column);
        columnIndices.push_back(columnIndex);
    }

    SetAffinity(0);

    auto cols = table.getColumns(columnIndices);

    while (state.KeepRunning())
    {
        for (ColumnPtr &col : cols)
        {
            col->scan();
        }
    }
}

static void BM_ColumnScan_1M_Rows__RemoteCols(benchmark::State& state) {
    Table table;

    std::vector<std::size_t> columnIndices;

    auto remoteColumns = state.range(0);

    auto rows = 1 * 1000 * 1000UL;

    for (auto i = 0; i < remoteColumns; ++i)
    {
        auto column = std::make_shared<Column<uint32_t>>(rows, numa_max_node());
        auto columnIndex = table.addColumn(column);
        columnIndices.push_back(columnIndex);
    }

    SetAffinity(0);

    auto cols = table.getColumns(columnIndices);

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
        {0, 8}, // Local columns
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_ColumnScan_1M_Rows__RemoteCols)
    ->RangeMultiplier(2)
    ->Ranges({
        {0, 8}  // Remote columns
    })
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
