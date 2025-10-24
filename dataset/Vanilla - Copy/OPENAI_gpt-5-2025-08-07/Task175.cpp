#include <bits/stdc++.h>
using namespace std;

struct MountainArray {
    virtual int get(int index) = 0;
    virtual int length() = 0;
    virtual ~MountainArray() {}
};

struct SimpleMountainArray : MountainArray {
    vector<int> arr;
    int calls;
    SimpleMountainArray(const vector<int>& a) : arr(a), calls(0) {}
    int get(int index) override {
        calls++;
        return arr[index];
    }
    int length() override {
        return (int)arr.size();
    }
};

int findInMountainArray(int target, MountainArray &mountainArr);

static int findPeak(MountainArray &m, int n) {
    int l = 0, r = n - 1;
    while (l < r) {
        int mid = l + (r - l) / 2;
        int a = m.get(mid);
        int b = m.get(mid + 1);
        if (a < b) l = mid + 1;
        else r = mid;
    }
    return l;
}

static int binarySearchAsc(MountainArray &m, int l, int r, int target) {
    while (l <= r) {
        int mid = l + (r - l) / 2;
        int val = m.get(mid);
        if (val == target) return mid;
        if (val < target) l = mid + 1;
        else r = mid - 1;
    }
    return -1;
}

static int binarySearchDesc(MountainArray &m, int l, int r, int target) {
    while (l <= r) {
        int mid = l + (r - l) / 2;
        int val = m.get(mid);
        if (val == target) return mid;
        if (val < target) r = mid - 1;
        else l = mid + 1;
    }
    return -1;
}

int findInMountainArray(int target, MountainArray &mountainArr) {
    int n = mountainArr.length();
    int peak = findPeak(mountainArr, n);
    int left = binarySearchAsc(mountainArr, 0, peak, target);
    if (left != -1) return left;
    return binarySearchDesc(mountainArr, peak + 1, n - 1, target);
}

int main() {
    vector<vector<int>> arrays = {
        {1,2,3,4,5,3,1},
        {0,1,2,4,2,1},
        {0,5,3,1},
        {0,2,4,5,3,1},
        {0,2,4,5,3,1}
    };
    vector<int> targets = {3, 3, 1, 0, 1};
    for (int i = 0; i < (int)arrays.size(); ++i) {
        SimpleMountainArray sma(arrays[i]);
        int res = findInMountainArray(targets[i], sma);
        cout << res << "\n";
    }
    return 0;
}