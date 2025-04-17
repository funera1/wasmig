cmake_minimum_required(VERSION 3.10)
project(WASMIG)

# ライブラリのヘッダーとソースを指定
add_library(libwasmig STATIC
    src/checkpoint.cpp
    src/restore.cpp
    src/table.cpp
    src/stack_tables.cpp
    src/log.cpp
    src/utils.cpp
    src/debug.cpp
)

# msgpackの設定
add_subdirectory(third_party/msgpack-c)
target_link_libraries(libwasmig PUBLIC msgpack-cxx)

# spdlogの設定
add_subdirectory(third_party/spdlog)
target_link_libraries(libwasmig PUBLIC spdlog::spdlog)

# wcrnの設定
add_subdirectory(wcrn)
target_link_libraries(libwasmig PUBLIC wcrn)

# protobufの設定
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
link_directories(${GTEST_LIBRARIES})
# add_subdirectory(third_party/protobuf)
add_subdirectory(third_party/protobuf-c/build-cmake)

# protoの設定
add_subdirectory(proto)
target_link_libraries(libwasmig PUBLIC proto_lib)

# ヘッダーファイルの公開ディレクトリを設定
target_include_directories(libwasmig 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
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