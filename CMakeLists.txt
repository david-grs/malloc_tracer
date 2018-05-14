cmake_minimum_required(VERSION 3.8)

project(stack_inspector)

set(CMAKE_CXX_STANDARD 17)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
	set(cxx_compile_options "-rdynamic -g -Wall -Wextra -Wpedantic -Wconversion -Wswitch-enum -Wunreachable-code -Wwrite-strings -Wcast-align -Wshadow -Wundef -Wold-style-cast -Wshadow -Wdouble-promotion")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cxx_compile_options}")

add_executable(stack_inspector backtrace.h main.cc)
target_link_libraries(stack_inspector dl)
