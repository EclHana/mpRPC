cmake_minimum_required(VERSION 3.0)
project(mpRpc)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 生成debug版本，可以进行gdb调
set(CMAKE_BUILD_TYPE "Debug")

# 头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)

# 链接库的搜索路径
link_directories(${PROJECT_SOURCE_DIR}/lib)

# src包含了mprpc框架所有相关代码
add_subdirectory(src)
# example包含了mprpc框架使用的示例代码
add_subdirectory(example)