#include <bits/stdc++.h>
using namespace std;

class MountainArray {
private:
    vector<int> data;
    int calls;
public:
    explicit MountainArray(const vector<int>& src) : data(src), calls(0) {}
    int get(int k) {
        if (k < 0 || k >= (int)data.size()) throw out_of_range("Index out of bounds");
        ++calls;
        return data[k];
    }
    int length() const {
        return (int)data.size();
    }
    int getCalls() const {
        return calls;
    }
};

int findPeak(MountainArray& m) {
    int lo = 0, hi = m.length() - 1;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        int a = m.get(mid);
        int b = m.get(mid + 1);
        if (a < b) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

int binAsc(MountainArray& m, int lo, int hi, int target) {
    int ans = -1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int val = m.get(mid);
        if (val == target) {
            ans = mid;
            hi = mid - 1;
        } else if (val < target) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return ans;
}

int binDesc(MountainArray& m, int lo, int hi, int target) {
    int ans = -1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int val = m.get(mid);
        if (val == target) {
            ans = mid;
            hi = mid - 1;
        } else if (val < target) {
            hi = mid - 1;
        } else {
            lo = mid + 1;
        }
    }
    return ans;
}

int findInMountainArray(int target, MountainArray& mountainArr) {
    int n = mountainArr.length();
    if (n < 3) return -1;
    int peak = findPeak(mountainArr);
    int left = binAsc(mountainArr, 0, peak, target);
    if (left != -1) return left;
    return binDesc(mountainArr, peak + 1, n - 1, target);
}

int findInMountainArrayWithRawArray(const vector<int>& arr, int target) {
    MountainArray m(arr);
    return findInMountainArray(target, m);
}

int main() {
    vector<pair<vector<int>, int>> tests = {
        {{1,2,3,4,5,3,1}, 3},
        {{0,1,2,4,2,1}, 3},
        {{0,5,3,1}, 1},
        {{1,2,3,4,5,4,3,2,1}, 9},
        {{0,1,0}, 0}
    };
    for (auto& tc : tests) {
        cout << findInMountainArrayWithRawArray(tc.first, tc.second) << "\n";
    }
    return 0;
}