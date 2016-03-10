#pragma once

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/mutex.h>
#include <bits/stdc++.h>
#include <algorithm>
#include "tbb/enumerable_thread_specific.h"
#include "types.h"

//This class regroups the elements stored in the ThreadLocal storage and stores them into them
//target_array, such that elements belonging to a group are stored consecutevely and such that
//the groups are stored in ascending order
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
