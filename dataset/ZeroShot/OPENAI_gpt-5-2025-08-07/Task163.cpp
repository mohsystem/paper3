#include <iostream>
#include <vector>
#include <string>

int longestRun(const std::vector<long long>& arr) {
    if (arr.empty()) return 0;
    int n = static_cast<int>(arr.size());
    int maxLen = 1;
    int curLen = 1;
    int dir = 0; // 1 inc, -1 dec, 0 none
    for (int i = 1; i < n; ++i) {
        long long diff = arr[i] - arr[i - 1];
        if (diff == 1) {
            if (dir == 1 || dir == 0) {
                ++curLen;
            } else {
                curLen = 2;
            }
            dir = 1;
        } else if (diff == -1) {
            if (dir == -1 || dir == 0) {
                ++curLen;
            } else {
                curLen = 2;
            }
            dir = -1;
        } else {
            if (curLen > maxLen) maxLen = curLen;
            curLen = 1;
            dir = 0;
        }
        if (curLen > maxLen) maxLen = curLen;
    }
    if (curLen > maxLen) maxLen = curLen;
    return maxLen;
}

static std::string vecToStr(const std::vector<long long>& v) {
    std::string s = "[";
    for (size_t i = 0; i < v.size(); ++i) {
        s += std::to_string(v[i]);
        if (i + 1 < v.size()) s += ", ";
    }
    s += "]";
    return s;
}

int main() {
    std::vector<std::vector<long long>> tests = {
        {1, 2, 3, 5, 6, 7, 8, 9},
        {1, 2, 3, 10, 11, 15},
        {5, 4, 2, 1},
        {3, 5, 7, 10, 15},
        {1}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i + 1) << " Input: " << vecToStr(tests[i])
                  << " -> Longest Run: " << longestRun(tests[i]) << "\n";
    }
    return 0;
}