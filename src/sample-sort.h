#ifndef PARALLEL_SAMPLE_SAMPLE_SORT_H_
#define PARALLEL_SAMPLE_SAMPLE_SORT_H_

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <bits/stdc++.h>
#include "tbb/enumerable_thread_specific.h"

#include "count-elements-per-group.h"
#include "regroup-elements-globally.h"
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
  template <typename number>
  static void printArray(std::vector<number> &arr){
    for(int i=0; i<arr.size();i++){
      std::cout << arr[i] << ",";
    }
    std::cout << std::endl;
  }


  template <typename number>
  class RecursiveParallelizer {
    public:
      RecursiveParallelizer(std::vector<number> &arr,
              std::vector<int> &group_offsets, int lo) :  arr_(arr),
                                               group_offsets(group_offsets), lo_ (lo) {

      }

      void operator()(const tbb::blocked_range<int>& range) const {
        for (int i = range.begin(); i != range.end(); ++i) {
          std::sort(arr_.begin() + lo_ + group_offsets[i], arr_.begin() + lo_ + group_offsets[i+1]);
        }
      }
    private:
      std::vector<int> &group_offsets;
      std::vector<number> &arr_;
      int lo_;
  };

  template <typename number>
  static void sort_(std::vector<number> &arr, int lo, int hi) {
    if (hi - lo + 1 <= 1)
        return;

    int pivot_cnt = pecount * sampleConst_;

    // we're about to select too many pivots or have only one thread? then just sort sequentially.
    if (pivot_cnt > hi - lo + 1 || pecount == 1) {
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
    GroupLocally <number>group_locally(arr, pivots, threadLocalGroups);
    tbb::parallel_for(tbb::blocked_range<int>(lo,hi+1), group_locally);

    //Second: Count size of different groups
    std::vector<int> group_counts(pecount,0);
    CountElementsPerGroup<number> count_elements_per_group(threadLocalGroups, group_counts);
    tbb::parallel_for(tbb::blocked_range<int>(0, pecount), count_elements_per_group);


    //Third: Calculate the position of the different groups in the original array
    std::vector<int>group_offsets(pecount+1, 0);
    for (int i = 1; i <= pecount; ++i) {
      group_offsets[i] = group_offsets[i-1] + group_counts[i-1];
    }
    
    //Fourth: Move groups into the original array
    RegroupElementsGlobally<number> regroup_elements_globally(arr, threadLocalGroups, group_offsets);
    tbb::parallel_for(tbb::blocked_range<int>(0, pecount), regroup_elements_globally);

    //Fifth: Sort elements locally
    RecursiveParallelizer<number>rec_call(arr, group_offsets, lo);
    parallel_for(tbb::blocked_range<int>(0, pecount), rec_call);

  }
} // namespace parallel_sample_sort


#endif // PARALLEL_SAMPLE_SAMPLE_SORT_H_
