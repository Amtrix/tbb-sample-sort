#ifndef PARALLEL_SAMPLE_SORT_REGROUPER_H_
#define PARALLEL_SAMPLE_SORT_REGROUPER_H_

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/mutex.h>
#include <bits/stdc++.h>
#include <algorithm>
#include "rank-counter.h"

//Used to regroup the elements of the array, such that the elements that belong to processor 0 are at the beginning of the new array,
//the elements of processor 1 follow and so on...
namespace parallel_sample_sort {
    template <typename number>
    class Regrouper {
      public:
        //original_array : array that shall be grouped
        //grouped_array: the resulting grouped array will be stored in here
        //groups: number of groups (i.e. number of processors)
        //ranks: the rank/group for every element of the original_array
        //offset: stores where the first element for each processor shall be stored (prefix sum over group sizes)
        Regrouper(std::vector<number>& original_array, std::vector<number>& grouped_array,
          int groups, std::vector<int>& ranks, std::vector<int>& offset)
            : original_array(original_array), grouped_array(grouped_array),
              groups(groups), ranks(ranks), offset(offset){

             mutexes = new tbb::mutex[groups];
             number_copied = new int[groups];
             for (int i = 0; i < groups; i++){
               number_copied[i] = 0;
             }
        }

        void operator()(const tbb::blocked_range<int>& range) const {
          for (int i = range.begin(); i != range.end(); ++i) {
            //Copy this element into the grouped_array, have to lock mutex for this
            int group = ranks[i];
            mutexes[group].lock();
            int dest = offset[group] + number_copied[group];
              grouped_array[dest] = original_array[i];
              number_copied[group]++;
            mutexes[group].unlock();
          }
        }

         void FreeMemory() {
           delete [] mutexes;
           delete [] number_copied;
         }
      private:
        std::vector<number>& original_array;
        std::vector<number>& grouped_array;
        int groups;
        std::vector<int>& ranks;
        std::vector<int>& offset;
        tbb::mutex* mutexes;
        int* number_copied;
    };

} // namespace parallel_sample_sort


#endif // PARALLEL_SAMPLE_SORT_REGROUPER_H_
