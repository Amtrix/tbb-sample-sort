#pragma once

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/mutex.h>
#include <bits/stdc++.h>
#include <algorithm>
#include "tbb/enumerable_thread_specific.h"
#include "types.h"

//This class groups the elements of the original array locally per Thread.
//The elements are stored in a ThreadLocal vector<vector<number>>(groups): One vector per group which stores all
//elements belonging to the group.
namespace parallel_sample_sort {
    template <typename number>
    class GroupLocally {
      public:

        GroupLocally(std::vector<number>& original_array, std::vector<number>& pivots,
        ThreadLocalGroupsType<number>& threadLocalGroups)
            : threadLocalGroups(threadLocalGroups), original_array(original_array), pivots(pivots){
        }

        int GetRank(number val) const {
          //this is basically std::lower_bound. Implementing it ourself is faster.
          int fdx = pivots.size();
          int count = pivots.size();
          int low = 0;
          while(count > 0){
            int middle = low + count/2;
            if(pivots[middle] <= val){
              low = middle+1;
              count -= count/2 + 1;
            }else{
              count = count/2;
            }
          }
          return low;
        }

        void operator()(const tbb::blocked_range<int>& range) const {
          typename ThreadLocalGroupsType<number>::reference local = threadLocalGroups.local();
          for (int i = range.begin(); i != range.end(); ++i) {
            int rank = GetRank(original_array[i]);
            local[rank].push_back(original_array[i]);
          }
        }
      private:
        std::vector<number>& original_array;
        std::vector<number>& pivots;
        ThreadLocalGroupsType<number>& threadLocalGroups;
    };

} // namespace parallel_sample_sort
