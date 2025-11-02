cmake_minimum_required(VERSION 3.10)
project(WASMIG)

# C++サポートの確認
option(WASMIG_USE_CPP "Use C++ implementation for table_v3" ON)

# ライブラリのソースファイルリスト
set(WASMIG_SOURCES
    src/checkpoint.cpp
    src/restore.cpp
    src/table.cpp
    src/stack_tables.cpp
    src/stack.cpp
    src/log.cpp
    src/utils.cpp
    src/state.cpp
    src/debug.cpp
    src/codepos.cpp
    src/table_v3.cpp
    src/registry.cpp
)

# ライブラリのヘッダーとソースを指定
# ★ libwasmig → wasmig に修正
add_library(wasmig STATIC ${WASMIG_SOURCES})

set_target_properties(wasmig PROPERTIES POSITION_INDEPENDENT_CODE ON)

# msgpack
if (NOT TARGET msgpack-cxx)
    add_subdirectory(third_party/msgpack-c)
endif()
target_link_libraries(wasmig PRIVATE msgpack-cxx)

# spdlog
if (NOT TARGET spdlog::spdlog)
    add_subdirectory(third_party/spdlog)
endif()
target_link_libraries(wasmig PRIVATE spdlog::spdlog)

# wcrn
if (NOT TARGET wcrn)
    add_subdirectory(wcrn)
endif()
target_link_libraries(wasmig PRIVATE wcrn)

# protobuf
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
link_directories(${GTEST_LIBRARIES})

if (NOT TARGET protobuf-c)
    add_subdirectory(third_party/protobuf-c/build-cmake)
endif()

# proto
add_subdirectory(proto)
target_link_libraries(wasmig PRIVATE proto_lib)

# include path
target_include_directories(wasmig 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/proto/include/
)

# ★ install も wasmig に修正
install(TARGETS wasmig
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)

# tests
option(ENABLE_TESTS "Enable unit tests" ON)
if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()