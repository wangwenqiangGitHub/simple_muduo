cmake_minimum_required(VERSION 3.0)

project(simple_muduo)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)
# set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "-std=c++11 -g")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(COMPILE_OPTIONS_DEFAULT
    "-fPIC"
    "-Wall;-Wextra"
    "-Wno-unused-function;-Wno-unused-parameter;-Wno-unused-variable"
	"-Wno-error=extra;-Wno-error=missing-field-initializers;-Wno-error=type-limits")
if(ENABLE_MEM_DEBUG)
  list(APPEND COMPILE_OPTIONS_DEFAULT
    "-Wl,-wrap,free;-Wl,-wrap,malloc;-Wl,-wrap,realloc;-Wl,-wrap,calloc")
  update_cached_list(MK_COMPILE_DEFINITIONS ENABLE_MEM_DEBUG)
  message(STATUS "已启用内存调试功能")
endif()

if(ENABLE_ASAN)
  list(APPEND COMPILE_OPTIONS_DEFAULT
    "-fsanitize=address;-fno-omit-frame-pointer")
  message(STATUS "已启用 Address Sanitize")
endif()

aux_source_directory(. SRC_LIST)
add_library(${PROJECT_NAME} SHARED ${SRC_LIST})

add_executable(testTcpServer ./examples/testTcpServer.cpp)
target_link_libraries(testTcpServer ${PROJECT_NAME})

add_executable(testTcpClient ./examples/testTcpClient.cpp)
target_link_libraries(testTcpClient ${PROJECT_NAME})

add_executable(testTimer ./examples/testTimer.cpp)
target_link_libraries(testTimer ${PROJECT_NAME})
target_compile_options(testTimer PRIVATE ${COMPILE_OPTIONS_DEFAULT})
