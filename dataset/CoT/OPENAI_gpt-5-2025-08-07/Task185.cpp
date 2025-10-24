#include <bits/stdc++.h>
using namespace std;

// Step 1: Problem Understanding + Step 2: Security Requirements:
// Find the median of two sorted arrays in O(log(m+n)) time.

double findMedianSortedArrays(const vector<int>& nums1, const vector<int>& nums2) {
    // Step 3: Secure Coding Generation: ensure searching on smaller array
    if (nums1.size() > nums2.size()) {
        return findMedianSortedArrays(nums2, nums1);
    }

    int m = static_cast<int>(nums1.size());
    int n = static_cast<int>(nums2.size());
    int totalLeft = (m + n + 1) / 2;

    int lo = 0, hi = m;
    while (lo <= hi) {
        int i = lo + (hi - lo) / 2;
        int j = totalLeft - i;

        int left1 = (i == 0) ? numeric_limits<int>::min() : nums1[i - 1];
        int right1 = (i == m) ? numeric_limits<int>::max() : nums1[i];
        int left2 = (j == 0) ? numeric_limits<int>::min() : nums2[j - 1];
        int right2 = (j == n) ? numeric_limits<int>::max() : nums2[j];

        if (left1 <= right2 && left2 <= right1) {
            if (((m + n) & 1) == 1) {
                return static_cast<double>(max(left1, left2));
            } else {
                return (static_cast<double>(max(left1, left2)) + static_cast<double>(min(right1, right2))) / 2.0;
            }
        } else if (left1 > right2) {
            hi = i - 1;
        } else {
            lo = i + 1;
        }
    }

    // Step 4: Code Review -> Fallback for unexpected state
    throw runtime_error("Invalid input: arrays must be sorted.");
}

int main() {
    // Step 5: Secure Code Output with 5 test cases
    vector<int> a1{1, 3}, b1{2};
    vector<int> a2{1, 2}, b2{3, 4};
    vector<int> a3{}, b3{1};
    vector<int> a4{0, 0}, b4{0, 0};
    vector<int> a5{-5, -3, -1}, b5{2, 4, 6, 8};

    cout.setf(std::ios::fixed); cout << setprecision(5);
    cout << findMedianSortedArrays(a1, b1) << "\n"; // 2.00000
    cout << findMedianSortedArrays(a2, b2) << "\n"; // 2.50000
    cout << findMedianSortedArrays(a3, b3) << "\n"; // 1.00000
    cout << findMedianSortedArrays(a4, b4) << "\n"; // 0.00000
    cout << findMedianSortedArrays(a5, b5) << "\n"; // 2.00000

    return 0;
}