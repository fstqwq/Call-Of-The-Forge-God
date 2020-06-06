cmake_minimum_required (VERSION 2.8)

project (forge)

add_compile_options(-Wall -Wextra -Wconversion -Wshadow -Wundef -O3)

add_executable(forge src/main.cpp src/util.cpp src/forge.cpp)
set_property(TARGET forge PROPERTY CXX_STANDARD 14)

