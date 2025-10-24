#include <bits/stdc++.h>
using namespace std;

double findMedianSortedArrays(const vector<int>& nums1, const vector<int>& nums2) {
    const vector<int> *A = &nums1, *B = &nums2;
    if (A->size() > B->size()) swap(A, B);
    int m = (int)A->size(), n = (int)B->size();
    int imin = 0, imax = m, half = (m + n + 1) / 2;
    while (imin <= imax) {
        int i = (imin + imax) / 2;
        int j = half - i;
        if (i < m && j > 0 && (*B)[j - 1] > (*A)[i]) {
            imin = i + 1;
        } else if (i > 0 && j < n && (*A)[i - 1] > (*B)[j]) {
            imax = i - 1;
        } else {
            int maxLeft;
            if (i == 0) maxLeft = (*B)[j - 1];
            else if (j == 0) maxLeft = (*A)[i - 1];
            else maxLeft = max((*A)[i - 1], (*B)[j - 1]);
            if (((m + n) & 1) == 1) return (double)maxLeft;
            int minRight;
            if (i == m) minRight = (*B)[j];
            else if (j == n) minRight = (*A)[i];
            else minRight = min((*A)[i], (*B)[j]);
            return ((double)maxLeft + (double)minRight) / 2.0;
        }
    }
    return 0.0;
}

int main() {
    cout.setf(std::ios::fixed); cout << setprecision(5);
    cout << findMedianSortedArrays(vector<int>{1,3}, vector<int>{2}) << "\n";
    cout << findMedianSortedArrays(vector<int>{1,2}, vector<int>{3,4}) << "\n";
    cout << findMedianSortedArrays(vector<int>{}, vector<int>{1}) << "\n";
    cout << findMedianSortedArrays(vector<int>{0,0}, vector<int>{0,0}) << "\n";
    cout << findMedianSortedArrays(vector<int>{2}, vector<int>{}) << "\n";
    return 0;
}