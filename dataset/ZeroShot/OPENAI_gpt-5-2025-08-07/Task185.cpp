#include <iostream>
#include <vector>
#include <limits>
#include <iomanip>
#include <algorithm>

class Task185 {
public:
    static double findMedianSortedArrays(const std::vector<int>& nums1, const std::vector<int>& nums2) {
        const std::vector<int> *A = &nums1, *B = &nums2;
        if (A->size() > B->size()) std::swap(A, B);

        int m = static_cast<int>(A->size());
        int n = static_cast<int>(B->size());
        int totalLeft = (m + n + 1) / 2;

        int lo = 0, hi = m;
        const int NEG_INF = std::numeric_limits<int>::min();
        const int POS_INF = std::numeric_limits<int>::max();

        while (lo <= hi) {
            int i = lo + (hi - lo) / 2;
            int j = totalLeft - i;

            int leftA = (i == 0) ? NEG_INF : (*A)[i - 1];
            int rightA = (i == m) ? POS_INF : (*A)[i];
            int leftB = (j == 0) ? NEG_INF : (*B)[j - 1];
            int rightB = (j == n) ? POS_INF : (*B)[j];

            if (leftA <= rightB && leftB <= rightA) {
                if (((m + n) & 1) == 1) {
                    return static_cast<double>(std::max(leftA, leftB));
                } else {
                    double a = static_cast<double>(std::max(leftA, leftB));
                    double b = static_cast<double>(std::min(rightA, rightB));
                    return (a + b) / 2.0;
                }
            } else if (leftA > rightB) {
                hi = i - 1;
            } else {
                lo = i + 1;
            }
        }
        throw std::invalid_argument("Invalid input arrays.");
    }
};

int main() {
    std::cout << std::fixed << std::setprecision(5);
    std::cout << Task185::findMedianSortedArrays({1,3}, {2}) << "\n";
    std::cout << Task185::findMedianSortedArrays({1,2}, {3,4}) << "\n";
    std::cout << Task185::findMedianSortedArrays({}, {1}) << "\n";
    std::cout << Task185::findMedianSortedArrays({0,0}, {0,0}) << "\n";
    std::cout << Task185::findMedianSortedArrays({2}, {}) << "\n";
    return 0;
}