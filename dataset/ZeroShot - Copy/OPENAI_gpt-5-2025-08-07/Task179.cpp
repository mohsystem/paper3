#include <iostream>
#include <vector>
#include <deque>

class Task179 {
public:
    static std::vector<int> maxSlidingWindow(const std::vector<int>& nums, int k) {
        if (k <= 0 || nums.empty() || k > static_cast<int>(nums.size())) {
            return {};
        }
        std::deque<int> dq;
        std::vector<int> res;
        res.reserve(nums.size() - k + 1);
        for (int i = 0; i < static_cast<int>(nums.size()); ++i) {
            while (!dq.empty() && dq.front() <= i - k) dq.pop_front();
            while (!dq.empty() && nums[dq.back()] <= nums[i]) dq.pop_back();
            dq.push_back(i);
            if (i >= k - 1) res.push_back(nums[dq.front()]);
        }
        return res;
    }
};

static void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ",";
        std::cout << v[i];
    }
    std::cout << "]\n";
}

int main() {
    std::vector<std::pair<std::vector<int>, int>> tests = {
        {{1,3,-1,-3,5,3,6,7}, 3},
        {{1}, 1},
        {{9,11}, 2},
        {{4,-2}, 1},
        {{7,2,4}, 2}
    };
    for (const auto& t : tests) {
        auto out = Task179::maxSlidingWindow(t.first, t.second);
        printVec(out);
    }
    return 0;
}