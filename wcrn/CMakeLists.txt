cmake_minimum_required(VERSION 3.15)
project(wcrn LANGUAGES C)

# オプション: examples をビルドするかどうか
option(BUILD_WCRN_EXAMPLES "Build wcrn examples" OFF)

# Rustライブラリの名前
set(RUST_LIB_NAME wcrn)

# Cargoのビルド出力先
set(CARGO_TARGET_DIR ${CMAKE_BINARY_DIR}/cargo)

# Rustライブラリをビルドするターゲット
add_custom_target(${RUST_LIB_NAME}_build ALL
    COMMAND cargo build --release --target-dir ${CARGO_TARGET_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building Rust library with Cargo"
)

# Rustのビルド成果物のパス
set(RUST_LIB_PATH ${CARGO_TARGET_DIR}/release/lib${RUST_LIB_NAME}.a)

# RustライブラリをCMakeのインポートライブラリとして定義
add_library(${RUST_LIB_NAME} STATIC IMPORTED GLOBAL)
add_dependencies(${RUST_LIB_NAME} ${RUST_LIB_NAME}_build)
set_target_properties(${RUST_LIB_NAME} PROPERTIES
    IMPORTED_LOCATION ${RUST_LIB_PATH}
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# examples/main.c をビルドする（必要な場合のみ）
if(BUILD_WCRN_EXAMPLES)
    add_executable(example examples/main.c)
    target_link_libraries(example PRIVATE ${RUST_LIB_NAME})
endif()