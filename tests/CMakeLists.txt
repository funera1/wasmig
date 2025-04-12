enable_testing()

# GoogleTest のソースを追加
add_subdirectory(third_party/googletest)

# テスト実行ファイルの作成
add_executable(
    my_tests
    test_hello.cpp
)

# テスト対象のライブラリと Google Test をリンク
target_link_libraries(my_tests
    PRIVATE
    libwasmig
    gtest_main
)

# CTest にテストを登録
include(GoogleTest)
gtest_discover_tests(my_tests)
