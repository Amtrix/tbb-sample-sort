#ifndef PARALLEL_SAMPLE_SORT_RANK_COUNTER_H_
#define PARALLEL_SAMPLE_SORT_RANK_COUNTER_H_

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <bits/stdc++.h>
#include <algorithm>

//Used to regroup the elements of the array, such that the elements that belong to processor 0 are at the beginning of the new array,
//the elements of processor 1 follow and so on...
namespace parallel_sample_sort {
    template <typename number>
    class Regrouper {
      public:
        RankCounter(std::vect)
            : targetArr_(targetArr), queries_(queries){

            sz_ = new std::atomic_int[targetArr.size()+1];
            for (int i = 0; i < targetArr.size() + 1; ++i)
                sz_[i] = 0;
        }

        int GetRank(number val) const {
          int fdx = targetArr_.size();
          int count = targetArr_.size();
          int low = 0;
          while(count > 0){
            int middle = low + count/2;
            if(targetArr_[middle] < val){
              low = middle+1;
              count -= count/2 + 1;
            }else{
              count = count/2;
            }
          }
          return low;
        }

        int GetRankCount(int dx) const {
            return sz_[dx];
        }

        void operator()(const tbb::blocked_range<int>& range) const {
          for (int i = range.begin(); i != range.end(); ++i) {
            sz_[GetRank(queries_[i])]++;
          }
        }

        void FreeMemory() {
          delete [] sz_;
        }
      private:
        std::vector<number> &targetArr_;
        std::vector<int> &queries_;
        std::atomic_int* sz_ = NULL;
    };

} // namespace parallel_sample_sort


#endif // PARALLEL_SAMPLE_SORT_RANK_COUNTER_H_
