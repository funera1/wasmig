cmake_minimum_required(VERSION 3.10)

# サンプル実行ファイルの作成
add_executable(
    table_v3_example
    table_v3_example.cpp
)

# ライブラリをリンク
target_link_libraries(table_v3_example
    PRIVATE
    wasmig
    spdlog::spdlog
)

# ヘッダーファイルのパスを設定
target_include_directories(table_v3_example
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)
