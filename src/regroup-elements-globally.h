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
    class RegroupElementsGlobally {
      public:

        RegroupElementsGlobally(std::vector<number>& target_array,
          ThreadLocalGroupsType<number>& threadLocalGroups, std::vector<int>& group_offsets)
            :target_array(target_array), threadLocalGroups(threadLocalGroups), group_offsets(group_offsets){
        }

        void operator()(const tbb::blocked_range<int>& range) const {
          for (int i = range.begin(); i != range.end(); ++i) {
            int added = 0;
            for(auto it = threadLocalGroups.begin(); it != threadLocalGroups.end(); it++){
              std::copy((*it)[i].begin(), (*it)[i].end(), target_array.begin() + group_offsets[i] + added);
              added += (*it)[i].size();
            }
          }
        }
      private:
        std::vector<number>& target_array;
        ThreadLocalGroupsType<number>& threadLocalGroups;
        std::vector<int>& group_offsets;
    };

} // namespace parallel_sample_sort
