#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/task_scheduler_init.h>
#include <bits/stdc++.h>
#include <gflags/gflags.h>
#include <chrono>
#include <atomic>

DEFINE_bool(random, false,
              "If the array should be filled with random values");

DEFINE_bool(fill_descending, false, "If the array should be filled with (n...1)");

DEFINE_int32(num_threads, -1, "Specifies the number of threads that should be used for the execution. " 
                              "Default: -1 for automatic.");

typedef int number;

namespace parallel_sample_sort {
  void sort_(std::vector<number> &arr, int lo, int hi);

  struct RankCnt {
    std::vector<number> *arr;
    std::vector<int> pivots;
    std::atomic_int* sz;

    int GetRank(number val) const {
      int fdx = pivots.size();
      for (int j = 0; j < pivots.size(); ++j) {
        if (val <= pivots[j]) {
          fdx = j;
          break;
        }
      }
      return fdx;
    }

    void operator()(const tbb::blocked_range<int>& range) const {
      for (int i = range.begin(); i != range.end(); ++i) {
        sz[GetRank(arr->at(i))]++;
      }
    }
  };

  struct ParallelRecursiveCall {
    RankCnt *ranker;
    int lo;
    std::vector<int> *pfx;

    void operator()(const tbb::blocked_range<int>& range) const {
      for (int i = range.begin(); i != range.end(); ++i) {
        sort_(*ranker->arr, lo + pfx->at(i), lo + pfx->at(i) + ranker->sz[i] - 1);
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

    // sort pivots
    std::sort(arr.begin() + lo, arr.begin() + lo + pivot_cnt);

    RankCnt rank_cnt;
    rank_cnt.pivots = std::vector<int>();

    // now select every sampleConst_-th for the final pivot sample;
    for (int i = 0; i < kNumPE; ++i)
      rank_cnt.pivots.push_back(arr[lo+i*sampleConst_]);

    
    rank_cnt.arr = &arr;
    rank_cnt.sz = new std::atomic_int[kNumPE+1];
    
    for (int i = 0; i < kNumPE+1; ++i)
      rank_cnt.sz[i] = 0;

    // we want to do stuff in-place with the grouping: find out how many elements go in each bucket
    parallel_for(tbb::blocked_range<int>(lo, hi + 1), rank_cnt);

    // now that we know the size of each bucket {rank_cnt.rank}, we can position
    // to pivots in their final position and move on getting all elements in their buckets
    
    // lets initialize the fill position
    std::vector<int>pfx(kNumPE+1, 0);
    for (int i = 1; i < kNumPE + 1; ++i)
      pfx[i] = pfx[i-1] + rank_cnt.sz[i-1];
    pfx.push_back(1e9); // dummy element for the right bound of the last group

    std::vector<int>added(kNumPE+1, 0);
    for (int i = lo; i < hi; ++i) {
      int r = rank_cnt.GetRank(arr[i]);

      while (i < lo+pfx[r] || i > lo+pfx[r] + added[r]) {
        std::swap(arr[lo+pfx[r] + added[r]], arr[i]);
        added[r]++;
        r = rank_cnt.GetRank(arr[i]);
      }
    }

    #ifdef DEBUG
      std::cout << "Rank size: " << " ";
      for (int i = 0; i < rank_cnt.pivots.size()+1; ++i)
        std::cout << rank_cnt.sz[i] << " "; std::cout << std::endl;

      std::cout << "Pivots: " << " ";
      for (int i = 0; i < rank_cnt.pivots.size(); ++i)
        std::cout << rank_cnt.pivots[i] << " "; std::cout<<std::endl;

      int cc = 0;
      for (int i = lo; i < hi; ++i) {
        if (arr[i] == rank_cnt.pivots[cc]) std::cout << "[";
        std::cout << arr[i];
        if (arr[i] == rank_cnt.pivots[cc]) { std::cout << "]"; cc++; }
          std::cout << " ";
      }std::cout<<std::endl;

   
      for (int i = 0; i < rank_cnt.pivots.size() + 1; ++i) {
        int piv = i < rank_cnt.pivots.size() ? rank_cnt.pivots[i] : 1e9;
        for (int k = lo + pfx[i]; k < lo + pfx[i] + rank_cnt.sz[i]; ++k)
          if (arr[k] > piv) {
            std::cout << "WRONG AT: " << i << ": " << piv << "; elem: " << k << std::endl;
          }
      }
    #endif

    
    ParallelRecursiveCall rec_call;
    rec_call.ranker = &rank_cnt;
    rec_call.lo = lo;
    rec_call.pfx = &pfx;

    parallel_for(tbb::blocked_range<int>(0, rank_cnt.pivots.size() + 1), rec_call);

    //for (int i = 0; i < rank_cnt.pivots.size() + 1; ++i)
   //   sort_(arr, lo + pfx[i], lo + pfx[i] + rank_cnt.sz[i] - 1);
      

    delete [] rank_cnt.sz;
  }

  void sort(std::vector<number> &arr) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
 
    sampleConst_ = std::log2(arr.size());
    sort_(arr, 0, ((int)arr.size()) - 1);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto usec = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Time: " << usec/1000000000.0 << std::endl;
  }


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
}

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

  if (FLAGS_num_threads != -1) {
    tbb::task_scheduler_init init(FLAGS_num_threads);
  }

  parallel_sample_sort::testRank();

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

  parallel_sample_sort::sort(arr);
  bool correctness = check_correctness(arr);

  if (correctness) std::cout << "Correct." << std::endl;
  else std::cout << "Incorrect resulting order." << std::endl;
}
