cmake_minimum_required(VERSION 3.10)

project(EMB_STL_CTime_Example)

add_executable(stl_ctime_example main.cpp)
target_include_directories(stl_ctime_example PRIVATE ../../include)
target_compile_features(stl_ctime_example PRIVATE cxx_std_11)