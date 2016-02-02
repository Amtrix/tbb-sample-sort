#ifndef PARALLEL_SAMPLE_SORT_RANK_COUNTER_H_
#define PARALLEL_SAMPLE_SORT_RANK_COUNTER_H_

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <bits/stdc++.h>

namespace parallel_sample_sort {
    template <typename number>
    class RankCounter {
      public:
        RankCounter(std::vector<int> &targetArr, std::vector<int> &queries)
            : targetArr_(targetArr), queries_(queries){

            sz_ = new std::atomic_int[targetArr.size()+1];
            for (int i = 0; i < targetArr.size() + 1; ++i)
                sz_[i] = 0;
        }

        int GetRank(number val) const {
          int fdx = targetArr_.size();
          for (int j = 0; j < targetArr_.size(); ++j) {
            if (val <= targetArr_[j]) {
              fdx = j;
              break;
            }
          }
          return fdx;
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