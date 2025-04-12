enable_testing()

# GoogleTest のソースを追加
add_subdirectory(third_party/googletest)

# テスト実行ファイルの作成
add_executable(
    wasmig_tests
    test_hello.cpp
)

# テスト対象のライブラリと Google Test をリンク
target_link_libraries(wasmig_tests
    PRIVATE
    libwasmig
    gtest_main
)

# test_dataディレクトリのパスをテストコードに渡す（後述の C++ 側で使う）
target_compile_definitions(wasmig_tests PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test_data"
)

# CTest にテストを登録
include(GoogleTest)
gtest_discover_tests(wasmig_tests)
