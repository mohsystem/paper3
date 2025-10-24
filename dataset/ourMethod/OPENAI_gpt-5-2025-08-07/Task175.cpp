#include <bits/stdc++.h>
using namespace std;

class MountainArray {
public:
    virtual int get(int index) = 0;
    virtual int length() const = 0;
    virtual int getCallCount() const = 0;
    virtual ~MountainArray() = default;
};

class ArrayMountain : public MountainArray {
private:
    vector<int> data;
    int calls;
public:
    explicit ArrayMountain(const vector<int>& arr) : data(arr), calls(0) {}
    int get(int index) override {
        if (index < 0 || index >= static_cast<int>(data.size())) {
            throw std::out_of_range("Index out of bounds");
        }
        calls++;
        return data[index];
    }
    int length() const override {
        return static_cast<int>(data.size());
    }
    int getCallCount() const override {
        return calls;
    }
};

int findPeak(MountainArray& arr) {
    int l = 0, r = arr.length() - 1;
    while (l < r) {
        int m = l + (r - l) / 2;
        int a = arr.get(m);
        int b = arr.get(m + 1);
        if (a < b) l = m + 1;
        else r = m;
    }
    return l;
}

int binaryAsc(MountainArray& arr, int l, int r, int target) {
    while (l <= r) {
        int m = l + (r - l) / 2;
        int v = arr.get(m);
        if (v == target) return m;
        if (v < target) l = m + 1;
        else r = m - 1;
    }
    return -1;
}

int binaryDesc(MountainArray& arr, int l, int r, int target) {
    while (l <= r) {
        int m = l + (r - l) / 2;
        int v = arr.get(m);
        if (v == target) return m;
        if (v < target) r = m - 1;
        else l = m + 1;
    }
    return -1;
}

int findInMountainArray(int target, MountainArray& mountainArr) {
    int n = mountainArr.length();
    if (n < 3) return -1;
    int peak = findPeak(mountainArr);
    int left = binaryAsc(mountainArr, 0, peak, target);
    if (left != -1) return left;
    return binaryDesc(mountainArr, peak + 1, n - 1, target);
}

void runTest(const vector<int>& array, int target, int expected) {
    ArrayMountain ma(array);
    int result = findInMountainArray(target, ma);
    cout << "Array: [";
    for (size_t i = 0; i < array.size(); ++i) {
        cout << array[i] << (i + 1 < array.size() ? "," : "");
    }
    cout << "], target=" << target
         << " -> result=" << result
         << ", expected=" << expected
         << ", getCalls=" << ma.getCallCount() << "\n";
}

int main() {
    runTest({1, 2, 3, 4, 5, 3, 1}, 3, 2);        // Example 1
    runTest({0, 1, 2, 4, 2, 1}, 3, -1);          // Example 2
    runTest({1, 3, 5, 7, 9, 6, 4, 2}, 1, 0);     // Target at start
    runTest({2, 5, 9, 12, 10, 7, 2}, 7, 5);      // Target on right
    runTest({0, 2, 5, 10, 9, 8, 1}, 10, 3);      // Target at peak
    return 0;
}