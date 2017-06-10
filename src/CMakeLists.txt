project(tuk_numa_benchmark)

include_directories(SYSTEM

)

set(sources
    main.cpp
)

add_executable(${PROJECT_NAME} ${sources})

target_link_libraries(${PROJECT_NAME}

)
