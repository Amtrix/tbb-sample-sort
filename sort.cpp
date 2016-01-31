#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <bits/stdc++.h>
#include <gflags/gflags.h>
#include <chrono>
#include <atomic>

DEFINE_bool(random, false,
              "If the array should be filled with random values");

typedef int number;

namespace parallel_sample_sort {
  struct RankCnt {
    const std::vector<number> *arr;
    std::atomic_int* rank;
    int pivot_cnt;
    int lo;

    void operator()(const tbb::blocked_range<int>& range) const {
      for (int i = range.begin(); i != range.end(); ++i) {
        number elem = arr->at(i);
        int fdx = pivot_cnt;
        for (int j = 0; j < pivot_cnt; ++j) {
          if (elem <= arr->at(lo+j)) {
            fdx = j;
            break;
          }
        }
        rank[fdx]++;
      }
    }
  };

  const int kNumPE = 8;

  int sampleConst_;

  void sort_(std::vector<number> &arr, int lo, int hi) {
    if (hi - lo + 1 <= 1)
        return;

    int pivot_cnt = kNumPE * sampleConst_;

    // we're about to select too many pivots? sort that right-away and return.
    if (pivot_cnt > hi - lo + 1) {
      std::sort(arr.begin() + lo, arr.begin() + hi + 1);
      return;
    }

    // lets select pivot_cnt random pivot elements and move them to the beginning of the array
    int elem_left = hi - lo + 1;
    for (int i = 0; i < pivot_cnt; ++i) {
      int rdx = rand() % elem_left;
      std::swap(arr[lo+i], arr[lo+i+rdx]);
      elem_left--;
    }

    // now select every sampleConst_-th for the final pivot sample;
    for (int i = 0; i < kNumPE; ++i)
      std::swap(arr[lo+i], arr[lo+i*sampleConst_]);

    // sort pivots
    std::sort(arr.begin() + lo, arr.begin() + lo + pivot_cnt);

    // we want to do stuff in-place with the grouping: find out how many elements go in each bucket
    RankCnt rank_cnt;
    rank_cnt.arr = &arr;
    rank_cnt.pivot_cnt = pivot_cnt;
    rank_cnt.lo = lo;
    rank_cnt.rank = new std::atomic_int[pivot_cnt+1];
    
    for (int i = 0; i < pivot_cnt+1; ++i)
      rank_cnt.rank[i] = 0;

    parallel_for(tbb::blocked_range<int>(lo + pivot_cnt, hi + 1), rank_cnt);

    // now that we know the size of each bucket {rank_cnt.rank}, we can position
    // to pivots in their final position and move on getting all elements in their buckets

    delete [] rank_cnt.rank;
  }

  void sort(std::vector<number> &arr) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
 
    sampleConst_ = std::log2(arr.size());
    sort_(arr, 0, ((int)arr.size()) - 1);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto usec = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Time: " << usec << std::endl;
  }


  void testRank() {
    std::vector<int> arr;
    arr.push_back(0);
    arr.push_back(10000);
    arr.push_back(1000000);
    for (int i = -10; i < 2000000; ++i)
      arr.push_back(i);

    RankCnt ranker;
    ranker.arr = &arr;
    ranker.pivot_cnt = 3;
    ranker.lo = 0;
    ranker.rank = new std::atomic_int[ranker.pivot_cnt+1];

    for (int i = 0; i < ranker.pivot_cnt+1; ++i)
        ranker.rank[i] = 0;

    parallel_for(tbb::blocked_range<int>(0 + ranker.pivot_cnt, arr.size()), ranker);

    for (int i = 0; i < ranker.pivot_cnt+1; ++i)
      std::cout << ranker.rank[i] << " ";
    std::cout << std::endl;

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

  //parallel_sample_sort::testRank();

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
