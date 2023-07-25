aux_source_directory(. SRC_LIST)

add_library(mpRpc ${SRC_LIST})

target_link_libraries(mpRpc muduo_net muduo_base pthread zookeeper_mt)