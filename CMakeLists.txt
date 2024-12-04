cmake_minimum_required(VERSION 3.10)
project(MyLibrary)

# ライブラリのヘッダーとソースを指定
add_library(migration STATIC
    src/migration.c
)

# ヘッダーファイルの公開ディレクトリを設定
target_include_directories(migration PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ライブラリのバージョン情報やインストール先の設定 (必要に応じて)
set_target_properties(migration PROPERTIES
    VERSION 1.0
    SOVERSION 1
)

# インストールターゲットの設定
install(TARGETS migration
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)
