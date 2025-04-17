# .protoファイルを指定
set(PROTO_FILES
    state.proto
)
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# protobuf-cのコンパイラを指定
set(PROTOC_EXECUTABLE /usr/local/bin/protoc)
set(PROTOC_GEN_C_EXECUTABLE /usr/local/bin/protoc-gen-c)
# find_program(PROTOC_EXECUTABLE protoc REQUIRED)
# find_program(PROTOC_GEN_C_EXECUTABLE protoc-gen-c REQUIRED)


# .protoファイルをコンパイルしてCコードを生成
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb-c.c ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb-c.h
        COMMAND ${PROTOC_EXECUTABLE}
        ARGS --proto_path=${PROTO_DIR}
             --plugin=protoc-gen-c=${PROTOC_GEN_C_EXECUTABLE}
             --c_out=${CMAKE_CURRENT_BINARY_DIR}
             ${PROTO_FILE}
        WORKING_DIRECTORY ${PROTO_DIR}  # ここがポイント！
        DEPENDS ${PROTO_DIR}/${PROTO_FILE}
    )
    list(APPEND PROTO_SRC_FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb-c.c)
endforeach()

# protobuf-cのライブラリをリンク
add_library(proto_lib STATIC ${PROTO_SRC_FILES})
target_include_directories(proto_lib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(proto_lib PUBLIC protobuf-c)