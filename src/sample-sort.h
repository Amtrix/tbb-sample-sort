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
  static void sort(std::vector<number> &arr, int number_of_threads) {
    pecount = number_of_threads;
    sampleConst_ = std::log2(arr.size());
    sort_<number>(arr, 0, ((int)arr.size()) - 1);
  }

  template <typename number>
  class RecursiveParallelizer {
    public:
      RecursiveParallelizer(RankCounter<number> &rank_cnt, std::vector<int> &arr,
              std::vector<int> &pfx, int lo) : rank_cnt_(rank_cnt), arr_(arr),
                                               pfx_(pfx), lo_ (lo) {

      }

      void operator()(const tbb::blocked_range<int>& range) const {
        for (int i = range.begin(); i != range.end(); ++i) {
          std::sort(arr_.begin() + lo_ + pfx_[i], arr_.begin() + lo_ + pfx_[i] + rank_cnt_.GetRankCount(i));
        }
      }
    private:
      RankCounter<number> &rank_cnt_;
      std::vector<int> &pfx_;
      std::vector<int> &arr_;
      int lo_;
  };

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

    std::vector<number> pivot_samples(pivot_cnt);
    // lets select pivot_cnt random pivot elements and store them in pivot_samples
    // do this in parallel
    int m = hi - lo + 1;
    tbb::parallel_for(tbb::blocked_range<size_t>(0,pivot_cnt),
      [m,lo,&arr,&pivot_samples](const tbb::blocked_range<size_t>& r) {
        for(size_t i = r.begin(); i != r.end(); i++){
          int rdx = rand() % m;
          pivot_samples[i] = arr[lo+rdx];
        }
      }
    );

    // sort pivots
    std::sort(pivot_samples.begin(), pivot_samples.end());


    std::vector<number> pivots;

    // now select every sampleConst_-th for the final pivot sample;
    for (int i = 1; i < pecount; ++i)
      pivots.push_back(pivot_samples[i*sampleConst_]);

    std::vector<int> ranks(arr.size());
    RankCounter<number>rank_cnt(ranks,pivots, arr);

    // we want to do stuff in-place with the grouping: find out how many elements go in each bucket
    parallel_for(tbb::blocked_range<int>(lo, hi + 1), rank_cnt);

    // now that we know the size of each bucket {rank_cnt.rank}, we can position
    // to pivots in their final position and move on getting all elements in their buckets

    // lets initialize the fill position
    std::vector<int>pfx(pecount, 0);
    for (int i = 1; i < pecount; ++i)
      pfx[i] = pfx[i-1] + rank_cnt.GetRankCount(i-1);
    pfx.push_back(1e9); // dummy element for the right bound of the last group

    std::vector<int>added(pecount, 0);
    for (int i = lo; i < hi; ++i) {
      int r = rank_cnt.GetRank(arr[i]);

      while (i < lo+pfx[r] || i > lo+pfx[r] + added[r]) {
        std::swap(arr[lo+pfx[r] + added[r]], arr[i]);
        added[r]++;
        r = rank_cnt.GetRank(arr[i]);
      }
    }

    RecursiveParallelizer<number>rec_call(rank_cnt, arr, pfx, lo);
    parallel_for(tbb::blocked_range<int>(0, pecount), rec_call);

    rank_cnt.FreeMemory();
  }
} // namespace parallel_sample_sort


#endif // PARALLEL_SAMPLE_SAMPLE_SORT_H_
