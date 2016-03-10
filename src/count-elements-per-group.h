#pragma once

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/mutex.h>
#include <bits/stdc++.h>
#include <algorithm>
#include "tbb/enumerable_thread_specific.h"
#include "types.h"

//This class counts the number of elements per group by iterating over all ThreadLocal vector<vecctor>s that were
//calculated in group-locally.
//The resulting number of elements per group will be stored in group_count
namespace parallel_sample_sort {
    template <typename number>
    class CountElementsPerGroup {
      public:

        CountElementsPerGroup(ThreadLocalGroupsType<number>& threadLocalGroups, std::vector<int>& group_counts)
            : threadLocalGroups(threadLocalGroups), group_counts(group_counts){
        }

        void operator()(const tbb::blocked_range<int>& range) const {
          for (int i = range.begin(); i != range.end(); ++i) {
            for(auto it = threadLocalGroups.begin(); it != threadLocalGroups.end(); it++){
              group_counts[i] += (*it)[i].size();
            }
          }
        }
      private:
        ThreadLocalGroupsType<number>& threadLocalGroups;
        std::vector<int>& group_counts;
    };

} // namespace parallel_sample_sort
