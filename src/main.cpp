#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/task_scheduler_init.h>
#include <bits/stdc++.h>
#include <gflags/gflags.h>
#include <chrono>
#include <atomic>
#include "sample-sort.h"

DEFINE_bool(random, false,
              "If the array should be filled with random values");

DEFINE_bool(fill_descending, false, "If the array should be filled with (n...1)");

DEFINE_int32(num_threads, -1, "Specifies the number of threads that should be used for the execution. " 
                              "Default: -1 for automatic.");

typedef int number;

/*
namespace parallel_sample_sort {
  void testRank() {
    std::vector<int> arr;
    int pcnt = 3;
    arr.push_back(0);
    arr.push_back(10000);
    arr.push_back(1000000);
    for (int i = -10; i < 2000000; ++i)
      arr.push_back(i);

    RankCnt ranker;
    ranker.pivots = std::vector<int>();
    for (int i = 0; i < pcnt; ++i)
      ranker.pivots.push_back(arr[i]);

    ranker.arr = &arr;
    ranker.sz = new std::atomic_int[pcnt+1];

    for (int i = 0; i < pcnt+1; ++i)
        ranker.sz[i] = 0;

    parallel_for(tbb::blocked_range<int>(0, arr.size()), ranker);

   // for (int i = 0; i < pcnt+1; ++i)
   //   std::cout << ranker.rank[i] << " ";
   // std::cout << std::endl;

    assert(ranker.sz[0] == 1+0 - (-10) + 1);
    assert(ranker.sz[1] == 1+10000 - 0);
    assert(ranker.sz[2] == 1+1000000 - 10000);
    assert(ranker.sz[3] == 2000000-1 - 1000000);
  }
}*/

bool check_correctness(const std::vector<number> &arr) {
  std::vector<number> stl_sorted(arr);

  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  std::sort(stl_sorted.begin(), stl_sorted.end());
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  auto usec = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "STL sort time: " << usec/1000000000.0 << std::endl;

  for (int i = 0; i + 1 < arr.size(); ++i)
      if (arr[i] != stl_sorted[i])
          return false;
  return true;
}


int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  std::ios_base::sync_with_stdio(false);

  std::cout << "Initialized with " << FLAGS_num_threads << " threads." << std::endl;
  tbb::task_scheduler_init init(FLAGS_num_threads);
  
  //parallel_sample_sort::testRank();

  number elem_count;
  std::cin >> elem_count;

  std::vector<number> arr;
  for (int i = 0; i < elem_count; ++i) {
      number num;
      if (FLAGS_random) num = rand();
      else if (FLAGS_fill_descending) num = elem_count - i;
      else std::cin >> num;
      arr.push_back(num);
  }

  parallel_sample_sort::sort<number>(arr);
  bool correctness = check_correctness(arr);

  if (correctness) std::cout << "Correct." << std::endl;
  else std::cout << "Incorrect resulting order." << std::endl;
}
