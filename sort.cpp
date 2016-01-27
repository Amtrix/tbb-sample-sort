#include <tbb/parallel_for.h>
#include <bits/stdc++.h>
#include <gflags/gflags.h>

DEFINE_bool(random, false,
              "If the array should be filled with random values");

typedef int number;

namespace parallel_sample_sort {

    const int kNumBuckets = 8;

    void _sort(std::vector<number> &arr, int lo, int hi) {
        if (hi - lo + 1 <= 1)
            return;

        int pivot_cnt = (hi - lo + 1) / kNumBuckets;


    }

    void sort(std::vector<number> &arr) {
        _sort(arr, 0, ((int)arr.size()) - 1);
    }
}

int main(int argc, char** argv) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    std::ios_base::sync_with_stdio(false);

    number elem_count;
    std::cin >> elem_count;

    std::vector<number> arr;
    for (int i = 0; i < elem_count; ++i) {
        number num;
        if (!FLAGS_random) std::cin >> num;
        else num = rand();
        arr.push_back(num);
    }

    parallel_sample_sort::sort(arr);

    bool correctness = true;
    for (int i = 0; i + 1 < arr.size(); ++i)
        if (!(arr[i] < arr[i+1]))
            correctness = false;

    if (correctness) std::cout << "Correct." << std::endl;
    else std::cout << "Incorrect resulting order." << std::endl;
}