cmake_minimum_required (VERSION 2.8)

project (forge)

add_compile_options(-Wall -Wextra -Wconversion -Wshadow -Wundef)

add_executable(forge main.cpp util.cpp)
set_property(TARGET forge PROPERTY CXX_STANDARD 11)

