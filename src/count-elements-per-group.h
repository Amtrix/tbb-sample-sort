#pragma once

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/mutex.h>
#include <bits/stdc++.h>
#include <algorithm>
#include "tbb/enumerable_thread_specific.h"
#include "types.h"

//Groups the elements of the original array locally.
//The elements are stored in a ThreadLocal vector<vector<number>>(groups): One vector per group in ThreadLocal
namespace parallel_sample_sort {
    template <typename number>
    class CountElementsPerGroup {
      public:

        CountElementsPerGroup(int number_of_groups, ThreadLocalGroupsType<number>& threadLocalGroups, std::vector<int>& group_counts)
            : number_of_groups(number_of_groups), threadLocalGroups(threadLocalGroups), group_counts(group_counts){
        }

        void operator()(const tbb::blocked_range<int>& range) const {
          for (int i = range.begin(); i != range.end(); ++i) {
            for(auto it = threadLocalGroups.begin(); it != threadLocalGroups.end(); it++){
              group_counts[i] += (*it)[i].size();
            }
          }
        }
      private:
        int number_of_groups;
        ThreadLocalGroupsType<number>& threadLocalGroups;
        std::vector<int>& group_counts;
    };

} // namespace parallel_sample_sort
