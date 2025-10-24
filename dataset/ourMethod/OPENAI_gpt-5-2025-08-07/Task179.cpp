#include <iostream>
#include <vector>
#include <deque>
#include <string>

std::vector<int> maxSlidingWindow(const std::vector<int>& nums, int k) {
    const int n = static_cast<int>(nums.size());
    if (n == 0 || k <= 0 || k > n) return {};
    std::deque<int> dq; // indices, values decreasing
    std::vector<int> res;
    res.reserve(n - k + 1);

    for (int i = 0; i < n; ++i) {
        while (!dq.empty() && dq.front() <= i - k) dq.pop_front();
        while (!dq.empty() && nums[dq.back()] <= nums[i]) dq.pop_back();
        dq.push_back(i);
        if (i >= k - 1) res.push_back(nums[dq.front()]);
    }
    return res;
}

void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ",";
        std::cout << v[i];
    }
    std::cout << "]";
}

void runTest(const std::vector<int>& nums, int k, const std::vector<int>& expected) {
    auto out = maxSlidingWindow(nums, k);
    std::cout << "nums="; printVec(nums); std::cout << ", k=" << k << "\n";
    std::cout << "out ="; printVec(out); std::cout << "\n";
    std::cout << "exp ="; printVec(expected); std::cout << "\n---\n";
}

int main() {
    runTest({1,3,-1,-3,5,3,6,7}, 3, {3,3,5,5,6,7});
    runTest({1}, 1, {1});
    runTest({9,8,7,6}, 4, {9});
    runTest({1,2,3,4,5}, 2, {2,3,4,5});
    runTest({5,4,3,2,1}, 2, {5,4,3,2});
    return 0;
}