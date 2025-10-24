#include <iostream>
#include <vector>
#include <stdexcept>
#include <limits>
#include <iomanip>
#include <algorithm>

double findMedianSortedArrays(const std::vector<int>& nums1, const std::vector<int>& nums2) {
    const std::vector<int>* A = &nums1;
    const std::vector<int>* B = &nums2;

    if (A->size() + B->size() == 0) {
        throw std::invalid_argument("Both input arrays are empty.");
    }

    if (A->size() > B->size()) {
        std::swap(A, B);
    }

    int m = static_cast<int>(A->size());
    int n = static_cast<int>(B->size());
    int low = 0, high = m;
    int half = (m + n + 1) / 2;

    while (low <= high) {
        int i = (low + high) / 2;
        int j = half - i;

        int Aleft  = (i == 0) ? std::numeric_limits<int>::min() : (*A)[i - 1];
        int Aright = (i == m) ? std::numeric_limits<int>::max() : (*A)[i];
        int Bleft  = (j == 0) ? std::numeric_limits<int>::min() : (*B)[j - 1];
        int Bright = (j == n) ? std::numeric_limits<int>::max() : (*B)[j];

        if (Aleft <= Bright && Bleft <= Aright) {
            if (((m + n) & 1) == 0) {
                long long leftMax = std::max(Aleft, Bleft);
                long long rightMin = std::min(Aright, Bright);
                return (leftMax + rightMin) / 2.0;
            } else {
                return static_cast<double>(std::max(Aleft, Bleft));
            }
        } else if (Aleft > Bright) {
            high = i - 1;
        } else {
            low = i + 1;
        }
    }

    throw std::invalid_argument("Input arrays must be sorted.");
}

static void runTest(const std::vector<int>& a, const std::vector<int>& b) {
    try {
        double res = findMedianSortedArrays(a, b);
        std::cout << std::fixed << std::setprecision(5) << res << "\n";
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << "\n";
    }
}

int main() {
    // 5 Test cases
    runTest({1, 3}, {2});                                       // 2.00000
    runTest({1, 2}, {3, 4});                                    // 2.50000
    runTest({}, {1});                                           // 1.00000
    runTest({0, 0}, {0, 0});                                    // 0.00000
    runTest({-1000000, -5, 0, 7}, {-3, 2, 2, 1000000});         // 1.00000
    return 0;
}