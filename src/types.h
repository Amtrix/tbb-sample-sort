#pragma once
template <typename number>
using ThreadLocalGroupsType = tbb::enumerable_thread_specific<std::vector<std::vector<number>>>;
