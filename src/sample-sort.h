#ifndef PARALLEL_SAMPLE_SAMPLE_SORT_H_
#define PARALLEL_SAMPLE_SAMPLE_SORT_H_

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <bits/stdc++.h>

#include "rank-counter.h"

namespace parallel_sample_sort {
   // not the best way to do it -> encapsulate together with sort_?
  static int pecount;
  static int sampleConst_;

  template <typename number>
  static void sort_(std::vector<number> &arr, int lo, int hi);

  template <typename number>
  class RecursiveParallelizer {
    public:
      RecursiveParallelizer(RankCounter<number> &rank_cnt, std::vector<int> &arr,
              std::vector<int> &pfx, int lo) : rank_cnt_(rank_cnt), arr_(arr),
                                               pfx_(pfx), lo_ (lo) {

      }

      void operator()(const tbb::blocked_range<int>& range) const {
        for (int i = range.begin(); i != range.end(); ++i) {
          sort_(arr_, lo_ + pfx_[i], lo_ + pfx_[i] + rank_cnt_.GetRankCount(i) - 1);
        }
      }
    private:
      RankCounter<number> &rank_cnt_;
      std::vector<int> &pfx_;
      std::vector<int> &arr_;
      int lo_;
  };

  template <typename number>
  static void sort(std::vector<number> &arr) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
 
    pecount = tbb::task_scheduler_init::default_num_threads();
    sampleConst_ = std::log2(arr.size());
    sort_<number>(arr, 0, ((int)arr.size()) - 1);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto usec = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Time: " << usec/1000000000.0 << std::endl;
  }

  template <typename number>
  static void sort_(std::vector<number> &arr, int lo, int hi) {
    if (hi - lo + 1 <= 1)
        return;

    int pivot_cnt = pecount * sampleConst_;

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

    
    std::vector<int> pivots;

    // now select every sampleConst_-th for the final pivot sample;
    for (int i = 0; i < pecount; ++i)
      pivots.push_back(arr[lo+i*sampleConst_]);

    
    RankCounter<number>rank_cnt(pivots, arr);

    // we want to do stuff in-place with the grouping: find out how many elements go in each bucket
    parallel_for(tbb::blocked_range<int>(lo, hi + 1), rank_cnt);

    // now that we know the size of each bucket {rank_cnt.rank}, we can position
    // to pivots in their final position and move on getting all elements in their buckets
    
    // lets initialize the fill position
    std::vector<int>pfx(pecount+1, 0);
    for (int i = 1; i < pecount + 1; ++i)
      pfx[i] = pfx[i-1] + rank_cnt.GetRankCount(i-1);
    pfx.push_back(1e9); // dummy element for the right bound of the last group

    std::vector<int>added(pecount+1, 0);
    for (int i = lo; i < hi; ++i) {
      int r = rank_cnt.GetRank(arr[i]);

      while (i < lo+pfx[r] || i > lo+pfx[r] + added[r]) {
        std::swap(arr[lo+pfx[r] + added[r]], arr[i]);
        added[r]++;
        r = rank_cnt.GetRank(arr[i]);
      }
    }

    RecursiveParallelizer<number>rec_call(rank_cnt, arr, pfx, lo);
    parallel_for(tbb::blocked_range<int>(0, pivots.size() + 1), rec_call);
    
    rank_cnt.FreeMemory();
  }
} // namespace parallel_sample_sort


#endif // PARALLEL_SAMPLE_SAMPLE_SORT_H_