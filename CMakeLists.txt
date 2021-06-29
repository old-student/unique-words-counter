cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(TestTask CXX)

find_package(Threads REQUIRED)

add_compile_options(-Wall -Wextra -Wpedantic)
add_executable(app main.cpp textprocessor.cpp threadpoolimpl.cpp)
target_compile_features(app PRIVATE cxx_std_17)
target_link_libraries(app Threads::Threads)
