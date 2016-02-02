cmake_minimum_required (VERSION 3.1.0 FATAL_ERROR)
project (Tbb-Sample-Sort)


add_executable (tbb-sample-sort main.cpp)
target_compile_features(tbb-sample-sort PRIVATE cxx_range_for)

target_link_libraries(tbb-sample-sort tbb)
target_link_libraries(tbb-sample-sort gflags)