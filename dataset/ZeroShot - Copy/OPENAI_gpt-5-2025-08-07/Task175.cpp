#include <iostream>
#include <vector>
#include <stdexcept>
#include <limits>

class MountainArray {
public:
    virtual int get(int k) = 0;
    virtual int length() = 0;
    virtual ~MountainArray() = default;
};

class TestMountainArray : public MountainArray {
    std::vector<int> arr;
    int getCount;
    int maxGets;
public:
    explicit TestMountainArray(const std::vector<int>& a, int maxGets_ = std::numeric_limits<int>::max())
        : arr(a), getCount(0), maxGets(maxGets_ < 1 ? 1 : maxGets_) {}

    int get(int k) override {
        if (k < 0 || k >= static_cast<int>(arr.size())) {
            throw std::out_of_range("Index out of bounds");
        }
        if (getCount + 1 > maxGets) {
            throw std::runtime_error("Exceeded get limit");
        }
        getCount++;
        return arr[static_cast<size_t>(k)];
    }

    int length() override {
        return static_cast<int>(arr.size());
    }

    int get_count() const { return getCount; }
};

int findPeak(MountainArray& m) {
    int lo = 0, hi = m.length() - 1;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        int a = m.get(mid);
        int b = m.get(mid + 1);
        if (a < b) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return lo;
}

int binSearchAsc(MountainArray& m, int lo, int hi, int target) {
    int res = -1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int val = m.get(mid);
        if (val == target) {
            res = mid;
            hi = mid - 1;
        } else if (val < target) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return res;
}

int binSearchDesc(MountainArray& m, int lo, int hi, int target) {
    int res = -1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int val = m.get(mid);
        if (val == target) {
            res = mid;
            hi = mid - 1;
        } else if (val > target) { // descending
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return res;
}

int findInMountainArray(int target, MountainArray& mountainArr) {
    int n = mountainArr.length();
    if (n < 3) return -1;
    int peak = findPeak(mountainArr);
    int left = binSearchAsc(mountainArr, 0, peak, target);
    if (left != -1) return left;
    return binSearchDesc(mountainArr, peak + 1, n - 1, target);
}

int main() {
    std::vector<std::pair<std::vector<int>, int>> tests = {
        {{1,2,3,4,5,3,1}, 3},
        {{0,1,2,4,2,1}, 3},
        {{0,5,3,1}, 1},
        {{1,5,2}, 2},
        {{1,2,3,4,5,6,7,6,5,4,3,2,1}, 7}
    };
    for (const auto& t : tests) {
        TestMountainArray m(t.first, 1000);
        int ans = findInMountainArray(t.second, m);
        std::cout << ans << "\n";
    }
    return 0;
}