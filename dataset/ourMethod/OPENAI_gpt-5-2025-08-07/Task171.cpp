#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdint>

static void validateArray(const std::vector<int>& nums) {
    std::size_t n = nums.size();
    if (n < 1 || n > 50000) {
        throw std::invalid_argument("Array length must be in the range [1, 50000].");
    }
    // Elements are int; no further range checks needed.
}

static long long sortAndCount(std::vector<int>& arr, std::vector<int>& temp, int left, int right) {
    if (left >= right) return 0LL;
    int mid = left + (right - left) / 2;
    long long cnt = 0LL;
    cnt += sortAndCount(arr, temp, left, mid);
    cnt += sortAndCount(arr, temp, mid + 1, right);

    int j = mid + 1;
    for (int i = left; i <= mid; ++i) {
        while (j <= right && static_cast<long long>(arr[i]) > 2LL * static_cast<long long>(arr[j])) {
            ++j;
        }
        cnt += (j - (mid + 1));
    }

    int i = left; j = mid + 1; int k = left;
    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    for (i = left; i <= right; ++i) arr[i] = temp[i];

    return cnt;
}

long long reversePairs(const std::vector<int>& nums) {
    validateArray(nums);
    if (nums.size() <= 1) return 0LL;
    std::vector<int> arr(nums.begin(), nums.end());
    std::vector<int> temp(nums.size());
    return sortAndCount(arr, temp, 0, static_cast<int>(arr.size()) - 1);
}

int main() {
    std::vector<std::vector<int>> tests = {
        {1, 3, 2, 3, 1},                 // expected 2
        {2, 4, 3, 5, 1},                 // expected 3
        {-5, -5},                        // expected 1
        {2147483647, -2147483648, 0},    // expected 2
        {1, 2, 3, 4, 5}                  // expected 0
    };

    for (std::size_t i = 0; i < tests.size(); ++i) {
        try {
            long long res = reversePairs(tests[i]);
            std::cout << "Test " << (i + 1) << " -> " << res << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Test " << (i + 1) << " error: " << e.what() << std::endl;
        }
    }
    return 0;
}