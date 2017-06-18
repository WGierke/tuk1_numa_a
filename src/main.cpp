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

static void BM_ColumnScan_1GB_Sequential(benchmark::State& state) {
    Table table;

    std::vector<std::size_t> columnIndices;

    auto localColumns = state.range(0);
    auto remoteColumns = state.range(1);

    auto rows = 1 * 1000 * 1000UL;

    for (auto i = 0; i < localColumns; ++i)
    {
        auto column = std::make_shared<Column<uint32_t>>(rows, 0);
        auto columnIndex = table.addColumn(column);
        columnIndices.push_back(columnIndex);
    }

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
BENCHMARK(BM_ColumnScan_1GB_Sequential)
    ->RangeMultiplier(2)
    ->Ranges({
        {1, 8}, // Local columns
        {1, 8}  // Remote columns
    })
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
