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

static void BM_ColumnScan_1GB_Sequential_Local_1_Columns(benchmark::State& state) {
    Table table;

    auto column = std::make_shared<Column<uint8_t>>(1 * 1000 * 1000 * 1000UL, 0);
    auto columnIndex = table.addColumn(column);

    SetAffinity(0);

    while (state.KeepRunning())
    {
        table.column(columnIndex)->scan();
    }
}
BENCHMARK(BM_ColumnScan_1GB_Sequential_Local_1_Columns)->Unit(benchmark::kMillisecond);

static void BM_ColumnScan_1GB_Sequential_Local_2_Columns(benchmark::State& state) {
    Table table;

    auto column1 = std::make_shared<Column<uint8_t>>(1 * 1000 * 1000 * 1000UL, 0);
    auto columnIndex1 = table.addColumn(column1);

    auto column2 = std::make_shared<Column<uint8_t>>(1 * 1000 * 1000 * 1000UL, 0);
    auto columnIndex2 = table.addColumn(column2);

    SetAffinity(0);

    std::vector<std::size_t> columnIndices = {columnIndex1, columnIndex2};
    auto cols = table.getColumns(columnIndices);

    while (state.KeepRunning())
    {
        for (ColumnPtr &col : cols)
        {
            col->scan();
        }
    }
}
BENCHMARK(BM_ColumnScan_1GB_Sequential_Local_2_Columns)->Unit(benchmark::kMillisecond);


static void BM_ColumnScan_1GB_Sequential_Local_4_Columns(benchmark::State& state) {
    Table table;

    std::vector<std::size_t> columnIndices;

    for (uint i = 0; i < 4; ++i)
    {
        auto column = std::make_shared<Column<uint8_t>>(1 * 1000 * 1000 * 1000UL, 0);
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
BENCHMARK(BM_ColumnScan_1GB_Sequential_Local_4_Columns)->Unit(benchmark::kMillisecond);

static void BM_ColumnScan_1GB_Sequential_Local_8_Columns(benchmark::State& state) {
    Table table;

    std::vector<std::size_t> columnIndices;

    for (uint i = 0; i < 8; ++i)
    {
        auto column = std::make_shared<Column<uint8_t>>(1 * 1000 * 1000 * 1000UL, 0);
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
BENCHMARK(BM_ColumnScan_1GB_Sequential_Local_8_Columns)->Unit(benchmark::kMillisecond);

static void BM_ColumnScan_1GB_Sequential_Remote_1_Columns(benchmark::State& state) {
    Table table;

    auto column = std::make_shared<Column<uint8_t>>(1 * 1000 * 1000 * 1000UL, numa_max_node());
    auto columnIndex = table.addColumn(column);

    SetAffinity(0);

    while (state.KeepRunning())
    {
        table.column(columnIndex)->scan();
    }
}
BENCHMARK(BM_ColumnScan_1GB_Sequential_Remote_1_Columns)->Unit(benchmark::kMillisecond);

static void BM_ColumnScan_1GB_Sequential_Remote_2_Columns(benchmark::State& state) {
  Table table;

  std::vector<std::size_t> columnIndices;

  for (uint i = 0; i < 2; ++i)
  {
      auto column = std::make_shared<Column<uint8_t>>(1 * 1000 * 1000 * 1000UL, numa_max_node());
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

BENCHMARK(BM_ColumnScan_1GB_Sequential_Remote_2_Columns)->Unit(benchmark::kMillisecond);

static void BM_ColumnScan_1GB_Sequential_Remote_4_Columns(benchmark::State& state) {
  Table table;

  std::vector<std::size_t> columnIndices;

  for (uint i = 0; i < 4; ++i)
  {
      auto column = std::make_shared<Column<uint8_t>>(1 * 1000 * 1000 * 1000UL, numa_max_node());
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

BENCHMARK(BM_ColumnScan_1GB_Sequential_Remote_4_Columns)->Unit(benchmark::kMillisecond);

static void BM_ColumnScan_1GB_Sequential_Remote_8_Columns(benchmark::State& state) {
  Table table;

  std::vector<std::size_t> columnIndices;

  for (uint i = 0; i < 8; ++i)
  {
      auto column = std::make_shared<Column<uint8_t>>(1 * 1000 * 1000 * 1000UL, numa_max_node());
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
BENCHMARK(BM_ColumnScan_1GB_Sequential_Remote_8_Columns)->Unit(benchmark::kMillisecond);

// static void BM_ColumnScan_1GB_Random_Local(benchmark::State& state) {
//     Table table;
//
//     auto column = std::make_shared<Column<uint8_t>>(1 * 1000 * 1000 * 1000UL, 0);
//     auto columnIndex = table.addColumn(column);
//
//     SetAffinity(0);
//
//     while (state.KeepRunning())
//     {
//         table.column(columnIndex)->scanRandom();
//     }
// }
//BENCHMARK(BM_ColumnScan_1GB_Random_Local)->Unit(benchmark::kMillisecond);


BENCHMARK_MAIN();
