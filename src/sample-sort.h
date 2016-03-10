#ifndef PARALLEL_SAMPLE_SAMPLE_SORT_H_
#define PARALLEL_SAMPLE_SAMPLE_SORT_H_

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <bits/stdc++.h>
#include "tbb/enumerable_thread_specific.h"

#include "rank-counter.h"
#include "count-elements-per-group.h"
#include "regroup-elements-globally.h"
#include "regrouper.h"
#include "group-locally.h"
#include "types.h"

namespace parallel_sample_sort {
   // not the best way to do it -> encapsulate together with sort_?
  static int pecount;
  static int sampleConst_;

  static const float EPS = 0.1; // with probability at least 1-1/m, no group will have more than (1+EPS)*(m/pecount) elements (see lecture)
                                // 0.1 seems to be a good value
  template <typename number>
  static void sort_(std::vector<number> &arr, int lo, int hi);

  template <typename number>
  static void sort(std::vector<number> &arr, int number_of_threads) {
    pecount = number_of_threads;
    sampleConst_ = 4.0/(EPS*EPS) * std::log(arr.size());
    sort_<number>(arr, 0, ((int)arr.size()) - 1);
  }

  static void printArray(std::vector<int> &arr){
    for(int i=0; i<arr.size();i++){
      std::cout << arr[i] << ",";
    }
    std::cout << std::endl;
  }


  template <typename number>
  class RecursiveParallelizer {
    public:
      RecursiveParallelizer(std::vector<int> &arr,
              std::vector<int> &pfx, int lo) :  arr_(arr),
                                               pfx_(pfx), lo_ (lo) {

      }

      void operator()(const tbb::blocked_range<int>& range) const {
        for (int i = range.begin(); i != range.end(); ++i) {
          std::sort(arr_.begin() + lo_ + pfx_[i], arr_.begin() + lo_ + pfx_[i+1]);
        }
      }
    private:
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

    ThreadLocalGroupsType<number> threadLocalGroups(//stores grouped elements per Thread
      [](){std::vector<std::vector<number>> v(pecount) ; return v;});

    //First: Each thread groups its elements by the target thread
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    GroupLocally <number>group_locally(arr, pivots, threadLocalGroups);
    tbb::parallel_for(tbb::blocked_range<int>(lo,hi+1), group_locally);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto usec = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "1. time: " << usec/1000000000.0 << std::endl;

    //Second: Count size of different groups
    start = std::chrono::steady_clock::now();

    std::vector<int> group_counts(pecount,0);
    CountElementsPerGroup<number> count_elements_per_group(pecount, threadLocalGroups, group_counts);

    tbb::parallel_for(tbb::blocked_range<int>(0, pecount), count_elements_per_group);

    end = std::chrono::steady_clock::now();
    usec = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "2. time: " << usec/1000000000.0 << std::endl;

    //Third: Calculate the position of the different groups in the original array
    start = std::chrono::steady_clock::now();

    std::vector<int>pfx(pecount+1, 0);
    for (int i = 1; i <= pecount; ++i) {
      pfx[i] = pfx[i-1] + group_counts[i-1];
    }
    end = std::chrono::steady_clock::now();
    usec = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "3. time: " << usec/1000000000.0 << std::endl;

    //Fourth: Move groups into the original array
    start = std::chrono::steady_clock::now();

    RegroupElementsGlobally<number> regroup_elements_globally(arr, threadLocalGroups, pfx);
    tbb::parallel_for(tbb::blocked_range<int>(0, pecount), regroup_elements_globally);

    end = std::chrono::steady_clock::now();
    usec = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "4. time: " << usec/1000000000.0 << std::endl;
    
    //Fifth: Sort elements locally
    start = std::chrono::steady_clock::now();

    RecursiveParallelizer<number>rec_call(arr, pfx, lo);
    parallel_for(tbb::blocked_range<int>(0, pecount), rec_call);

    end = std::chrono::steady_clock::now();
    usec = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "5. time: " << usec/1000000000.0 << std::endl;
  }
} // namespace parallel_sample_sort


#endif // PARALLEL_SAMPLE_SAMPLE_SORT_H_
