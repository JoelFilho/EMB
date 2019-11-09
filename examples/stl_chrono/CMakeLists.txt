cmake_minimum_required(VERSION 3.10)

project(EMB_STL_Example)

add_executable(stl_example main.cpp)
target_include_directories(stl_example PRIVATE ../../include)
target_compile_features(stl_example PRIVATE cxx_std_11)