#include <iostream>
#include <vector>

struct OutlierResult {
    bool success;
    long long value;
};

static inline bool is_even(long long x) {
    return (x & 1LL) == 0;
}

OutlierResult findOutlier(const std::vector<long long>& arr) {
    OutlierResult res{false, 0};
    if (arr.size() < 3) {
        return res; // fail closed
    }

    int even_count = 0;
    int odd_count = 0;
    for (size_t i = 0; i < 3 && i < arr.size(); ++i) {
        if (is_even(arr[i])) {
            ++even_count;
        } else {
            ++odd_count;
        }
    }

    bool majority_even = even_count >= 2;

    long long candidate = 0;
    size_t outliers_found = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        bool curr_even = is_even(arr[i]);
        if (curr_even != majority_even) {
            candidate = arr[i];
            ++outliers_found;
            if (outliers_found > 1) {
                return res; // multiple outliers found -> fail
            }
        }
    }

    if (outliers_found == 1) {
        res.success = true;
        res.value = candidate;
    }
    return res;
}

int main() {
    std::vector<std::vector<long long>> tests = {
        {2, 4, 0, 100, 4, 11, 2602, 36},           // outlier: 11
        {160, 3, 1719, 19, 11, 13, -21},           // outlier: 160
        {2, 6, 8, -10, 12, -14, 15, 16, 18, 20},   // outlier: 15
        {1, 3, 5, 7, 4, 9, 11},                    // outlier: 4
        {-1, -3, -5, -7, -9, -1001, -2002}         // outlier: -2002
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        OutlierResult r = findOutlier(tests[i]);
        if (r.success) {
            std::cout << "Test " << (i + 1) << " outlier: " << r.value << "\n";
        } else {
            std::cout << "Test " << (i + 1) << " error: could not determine outlier\n";
        }
    }
    return 0;
}