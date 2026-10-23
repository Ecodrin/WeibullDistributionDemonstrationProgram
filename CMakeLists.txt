cmake_minimum_required(VERSION 3.22)
project(OC LANGUAGES CXX)

add_subdirectory(libs)

add_executable(main main.cpp)
target_link_libraries(main weibull_distribution_math_library)
target_link_libraries(main fuzzy_logic_library)