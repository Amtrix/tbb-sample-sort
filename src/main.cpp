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
              "If the array should be filled with random values");

DEFINE_bool(fill_descending, false, "If the array should be filled with (n...1)");

DEFINE_int32(num_threads, -1, "Specifies the number of threads that should be used for the execution. "
                              "Default: -1 for automatic.");

typedef int number;

bool check_correctness(const std::vector<number> &arr, const std::vector<number>& parallel_sorted) {
  std::vector<number> stl_sorted(arr);

  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  std::sort(stl_sorted.begin(), stl_sorted.end());
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  auto usec = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "STL sort time: " << usec/1000000000.0 << std::endl;

  for (int i = 0; i + 1 < arr.size(); ++i)
      if (parallel_sorted[i] != stl_sorted[i])
          return false;
  return true;
}

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  std::ios_base::sync_with_stdio(false);

  std::cout << "Initialized with " << FLAGS_num_threads << " threads." << std::endl;
  tbb::task_scheduler_init init(FLAGS_num_threads);
  int p = FLAGS_num_threads;
  if(FLAGS_num_threads == -1){
    p = tbb::task_scheduler_init::default_num_threads();
  }

  number elem_count;
  std::cin >> elem_count;
  std::srand(std::time(0)); // use current time as seed for random generator
  std::vector<number> arr;
  for (int i = 0; i < elem_count; ++i) {
      number num;
      if (FLAGS_random) num = rand();
      else if (FLAGS_fill_descending) num = elem_count - i;
      else std::cin >> num;
      arr.push_back(num);
  }
  std::vector<number> parallel_sorted(arr);
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  parallel_sample_sort::sort<number>(parallel_sorted,p);
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  auto usec = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "Time: " << usec/1000000000.0 << std::endl;

  bool correctness = check_correctness(arr, parallel_sorted);

  if (correctness) std::cout << "Correct." << std::endl;
  else std::cout << "Incorrect resulting order." << std::endl;
}
