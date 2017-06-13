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

static void BM_ColumnScan_1GB_Sequential_Local(benchmark::State& state) {
    Table table;

    auto column = std::make_shared<Column<uint8_t>>(1 * 1000 * 1000 * 1000UL, 0);
    auto columnIndex = table.addColumn(column);

    SetAffinity(0);

    while (state.KeepRunning())
    {
        table.column(columnIndex)->scan();
    }
}
BENCHMARK(BM_ColumnScan_1GB_Sequential_Local)->Unit(benchmark::kMillisecond);

static void BM_ColumnScan_1GB_Sequential_Remote(benchmark::State& state) {
    Table table;

    auto column = std::make_shared<Column<uint8_t>>(1 * 1000 * 1000 * 1000UL, numa_max_node());
    auto columnIndex = table.addColumn(column);

    SetAffinity(0);

    while (state.KeepRunning())
    {
        table.column(columnIndex)->scan();
    }
}
BENCHMARK(BM_ColumnScan_1GB_Sequential_Remote)->Unit(benchmark::kMillisecond);

static void BM_ColumnScan_1GB_Random_Local(benchmark::State& state) {
    Table table;

    auto column = std::make_shared<Column<uint8_t>>(1 * 1000 * 1000 * 1000UL, 0);
    auto columnIndex = table.addColumn(column);

    SetAffinity(0);

    while (state.KeepRunning())
    {
        table.column(columnIndex)->scanRandom();
    }
}
BENCHMARK(BM_ColumnScan_1GB_Random_Local)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
