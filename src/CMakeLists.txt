project(tuk_numa_benchmark)

set(SOURCES
    column.cpp
    column.h
    main.cpp
    numaalloc.h
    random.cpp
    random.h
    table.cpp
    table.h
    table.hpp
    tablegenerator.cpp
    tablegenerator.h)

# Global flags and include directories
add_compile_options(-std=c++11 -pthread -Wno-error=unused-parameter -Wall -Werror)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${NUMA_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/third_party/benchmark/include
)

target_link_libraries(${PROJECT_NAME}
    ${NUMA_LIBRARY}
    benchmark
)
