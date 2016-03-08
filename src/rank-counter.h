#ifndef PARALLEL_SAMPLE_SORT_RANK_COUNTER_H_
#define PARALLEL_SAMPLE_SORT_RANK_COUNTER_H_

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <bits/stdc++.h>
#include <algorithm>

//Used to calculate the amount of elements per processor. (accessible by GetRankCount(proccesor_number))
//It also calculates (stores in ranks) to which processor an element shall be sent
namespace parallel_sample_sort {
    template <typename number>
    class RankCounter {
      public:
        RankCounter(std::vector<int>& ranks, std::vector<int> &targetArr, std::vector<number> &queries)
            : ranks(ranks), targetArr_(targetArr), queries_(queries){

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
            int rank = GetRank(queries_[i]);
            sz_[rank]++;
            ranks[i] = rank;
          }
        }

        void FreeMemory() {
          delete [] sz_;
        }

      std::vector<int>& ranks;
      private:
        std::vector<number> &targetArr_;
        std::vector<number> &queries_;
        std::atomic_int* sz_ = NULL;
    };

} // namespace parallel_sample_sort


#endif // PARALLEL_SAMPLE_SORT_RANK_COUNTER_H_
