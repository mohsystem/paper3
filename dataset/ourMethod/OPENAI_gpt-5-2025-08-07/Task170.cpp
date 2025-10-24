#include <iostream>
#include <vector>
#include <stdexcept>
#include <limits>

static long long sort_count(std::vector<long long>& sums, size_t left, size_t right, long long lower, long long upper, std::vector<long long>& temp) {
    if (right - left <= 1) return 0LL;
    size_t mid = left + (right - left) / 2;
    long long cnt = 0;
    cnt += sort_count(sums, left, mid, lower, upper, temp);
    cnt += sort_count(sums, mid, right, lower, upper, temp);

    size_t lo = mid, hi = mid;
    for (size_t i = left; i < mid; ++i) {
        while (lo < right && sums[lo] - sums[i] < lower) ++lo;
        while (hi < right && sums[hi] - sums[i] <= upper) ++hi;
        cnt += static_cast<long long>(hi - lo);
    }
    // merge
    size_t i = left, j = mid, k = left;
    while (i < mid || j < right) {
        if (j >= right || (i < mid && sums[i] <= sums[j])) {
            temp[k++] = sums[i++];
        } else {
            temp[k++] = sums[j++];
        }
    }
    for (size_t t = left; t < right; ++t) {
        sums[t] = temp[t];
    }
    return cnt;
}

int countRangeSum(const std::vector<int>& nums, long long lower, long long upper) {
    if (nums.empty()) {
        throw std::invalid_argument("nums must not be empty");
    }
    if (nums.size() > 100000ULL) {
        throw std::invalid_argument("nums length out of allowed range");
    }
    if (lower > upper) {
        throw std::invalid_argument("lower must be <= upper");
    }
    if (lower < -100000LL || upper > 100000LL) {
        throw std::invalid_argument("lower/upper out of allowed range");
    }
    std::vector<long long> sums(nums.size() + 1, 0LL);
    for (size_t i = 0; i < nums.size(); ++i) {
        sums[i + 1] = sums[i] + static_cast<long long>(nums[i]);
    }
    std::vector<long long> temp(sums.size(), 0LL);
    long long cnt = sort_count(sums, 0, sums.size(), lower, upper, temp);
    if (cnt < std::numeric_limits<int>::min() || cnt > std::numeric_limits<int>::max()) {
        throw std::invalid_argument("Result out of 32-bit integer range");
    }
    return static_cast<int>(cnt);
}

static void runTest(const std::vector<int>& nums, long long lower, long long upper, int expected) {
    int result = countRangeSum(nums, lower, upper);
    std::cout << "Result: " << result << " | Expected: " << expected << "\n";
}

int main() {
    // Five test cases
    runTest(std::vector<int>{-2, 5, -1}, -2, 2, 3);
    runTest(std::vector<int>{0}, 0, 0, 1);
    runTest(std::vector<int>{1, 2, 3}, 3, 6, 4);
    runTest(std::vector<int>{-1, -1, -1}, -3, -1, 6);
    runTest(std::vector<int>{1, -1, 1, -1}, 0, 0, 4);
    return 0;
}