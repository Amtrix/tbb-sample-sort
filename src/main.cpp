#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/task_scheduler_init.h>
#include <bits/stdc++.h>
#include <gflags/gflags.h>
#include <chrono>
#include <atomic>
#include "sample-sort.h"
#include <ctime>

DEFINE_bool(random, false,
              "If the array should be filled with random values.");

DEFINE_bool(fill_zero, false, "If the array should be filled with 0s");

DEFINE_bool(use_floats, false, "If the generated numbers should be floats (int otherwise)");

DEFINE_int32(num_threads, -1, "Specifies the number of threads that should be used for the execution. "
                              "Default: -1 for automatic.");

DEFINE_int32(num_elements, 100000, "Specifies the number of elements that shall be sorted.");

void printResultHelper(std::string algo, float sec, int p) {
  std::string elementtype = "int";
  if(FLAGS_use_floats) {
    elementtype = "float";
  }
  std::string generator = "zero";
  if(FLAGS_random) {
    generator = "random";
  }
  std::cout << "RESULT"
          << " algo=" << algo
          << " time=" << sec
          << " size=" << FLAGS_num_elements
          << " generator=" << generator
          << " elementtype=" << elementtype
          << " threads=" << p
          << std::endl;
}

void printResult(float sec_stl, float sec_parallel, int p) {
  printResultHelper("sample_sort", sec_parallel, p);
  printResultHelper("std::sort", sec_stl, p);
}

template <typename number>
bool check_correctness(const std::vector<number> &stl_sorted, const std::vector<number>& parallel_sorted) {
  for (int i = 0; i + 1 < stl_sorted.size(); ++i)
      if (parallel_sorted[i] != stl_sorted[i])
          return false;
  return true;
}
template <typename number>
void run() {
  std::cout << "Initialized with " << FLAGS_num_threads << " threads." << std::endl;
  tbb::task_scheduler_init init(FLAGS_num_threads);
  int p = FLAGS_num_threads;
  if(FLAGS_num_threads == -1){
    p = tbb::task_scheduler_init::default_num_threads();
  }

  number elem_count = FLAGS_num_elements;
  std::srand(std::time(0)); // use current time as seed for random generator
  std::vector<number> arr;
  for (int i = 0; i < elem_count; ++i) {
      number num;
      if (FLAGS_random) {
        num = rand();
        if( FLAGS_use_floats) {
          num  = num/(1.0f * RAND_MAX)  * (1e10);
        }
      }
      else if (FLAGS_fill_zero) {
        num = 0;
      }
      else {
        std::cerr << "Missing parameter for the type of array values (random or zero)." << std::endl;
        return;
      }
      arr.push_back(num);
  }
  std::vector<number> parallel_sorted(arr);
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  parallel_sample_sort::sort<number>(parallel_sorted,p);
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  float sec_parallel = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()/1000000000.0;
  std::cout << "Time: " << sec_parallel << std::endl;


  std::vector<number> stl_sorted(arr);

  start = std::chrono::steady_clock::now();
  std::sort(stl_sorted.begin(), stl_sorted.end());
  end = std::chrono::steady_clock::now();
  float sec_stl = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()/1000000000.0;
  std::cout << "STL sort time: " << sec_stl << std::endl;

  bool correctness = check_correctness(stl_sorted, parallel_sorted);

  if (correctness) {
    std::cout << "Correct." << std::endl;
    printResult(sec_stl, sec_parallel, p);
  }
  else std::cout << "Incorrect resulting order." << std::endl;
}

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  std::ios_base::sync_with_stdio(false);

  if(FLAGS_use_floats){
    run<float>();
  } else {
    run<int>();
  }
}
