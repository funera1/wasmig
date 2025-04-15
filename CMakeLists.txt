cmake_minimum_required(VERSION 3.10)
project(WASMIG)

# ライブラリのヘッダーとソースを指定
add_library(libwasmig STATIC
    src/migration.cpp
    src/table.cpp
    src/stack_tables.cpp
)

# msgpackの設定
add_subdirectory(third_party/msgpack-c)
target_link_libraries(libwasmig PUBLIC msgpack-cxx)

# spdlogの設定
add_subdirectory(third_party/spdlog)
target_link_libraries(libwasmig PUBLIC spdlog::spdlog)

# spdlogの設定
add_subdirectory(third_party/wcrn)
target_link_libraries(libwasmig PUBLIC wcrn)

# ヘッダーファイルの公開ディレクトリを設定
target_include_directories(libwasmig 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# インストールターゲットの設定
install(TARGETS libwasmig
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)

# tests
option(ENABLE_TESTS "Enable unit tests" ON)
if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()