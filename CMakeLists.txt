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
)

# table_v3の実装選択
if(WASMIG_USE_CPP)
    list(APPEND WASMIG_SOURCES src/table_v3.cpp)
    message(STATUS "Using C++ implementation for table_v3")
else()
    list(APPEND WASMIG_SOURCES src/table_v3_c_impl.c)
    message(STATUS "Using C implementation for table_v3")
endif()

# ライブラリのヘッダーとソースを指定
add_library(libwasmig STATIC ${WASMIG_SOURCES})

set_target_properties(libwasmig PROPERTIES POSITION_INDEPENDENT_CODE ON)

# msgpackの設定
if (NOT TARGET msgpack-cxx)
    add_subdirectory(third_party/msgpack-c)
endif()
target_link_libraries(libwasmig PRIVATE msgpack-cxx)

# spdlogの設定
if (NOT TARGET spdlog::spdlog)
    add_subdirectory(third_party/spdlog)
endif()
target_link_libraries(libwasmig PRIVATE spdlog::spdlog)

# wcrnの設定
if (NOT TARGET wcrn)
    add_subdirectory(wcrn)
endif()
target_link_libraries(libwasmig PRIVATE wcrn)

# protobufの設定
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
link_directories(${GTEST_LIBRARIES})
# add_subdirectory(third_party/protobuf)
if (NOT TARGET protobuf-c)
    add_subdirectory(third_party/protobuf-c/build-cmake)
endif()

# protoの設定
add_subdirectory(proto)
target_link_libraries(libwasmig PRIVATE proto_lib)

# ヘッダーファイルの公開ディレクトリを設定
target_include_directories(libwasmig 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/proto/include/
)

# インストールターゲットの設定
install(TARGETS libwasmig
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
link_directories(${GTEST_LIBRARIES})
# tests
option(ENABLE_TESTS "Enable unit tests" ON)
if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# examples
option(ENABLE_EXAMPLES "Enable examples" ON)
if (ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()