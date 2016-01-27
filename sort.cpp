#include <tbb/parallel_for.h>
#include <bits/stdc++.h>
#include <gflags/gflags.h>
#include <chrono>

DEFINE_bool(random, false,
              "If the array should be filled with random values");

typedef int number;

namespace parallel_sample_sort {

    const int kNumBuckets = 8;

    void _sort(std::vector<number> &arr, int lo, int hi) {
        if (hi - lo + 1 <= 1)
            return;

        int pivot_cnt = (hi - lo + 1) / kNumBuckets;


    }

    void sort(std::vector<number> &arr) {
      std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        _sort(arr, 0, ((int)arr.size()) - 1);
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      auto usec = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
      std::cout << "Time: " << usec << std::endl;
    }
}

bool check_correctness(const std::vector<number> &arr) {
  std::vector<number> stl_sorted(arr);
  std::sort(stl_sorted.begin(), stl_sorted.end());
  for (int i = 0; i + 1 < arr.size(); ++i)
      if (arr[i] != stl_sorted[i])
          return false;
  return true;
}

int main(int argc, char** argv) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    std::ios_base::sync_with_stdio(false);

    number elem_count;
    std::cin >> elem_count;

    std::vector<number> arr;
    for (int i = 0; i < elem_count; ++i) {
        number num;
        if (!FLAGS_random) std::cin >> num;
        else num = rand();
        arr.push_back(num);
    }

    parallel_sample_sort::sort(arr);
    bool correctness = check_correctness(arr);

    if (correctness) std::cout << "Correct." << std::endl;
    else std::cout << "Incorrect resulting order." << std::endl;
}
