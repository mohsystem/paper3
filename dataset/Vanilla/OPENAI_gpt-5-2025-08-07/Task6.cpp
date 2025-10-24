#include <iostream>
#include <vector>
using namespace std;

double findUniq(const vector<double>& arr) {
    if (arr.size() < 3) throw invalid_argument("Array must have at least 3 elements");
    double a = arr[0], b = arr[1], c = arr[2];
    double common = (a == b || a == c) ? a : b;
    for (double v : arr) {
        if (v != common) return v;
    }
    return common; // Fallback
}

int main() {
    vector<vector<double>> tests = {
        {1, 1, 1, 2, 1, 1},
        {0, 0, 0.55, 0, 0},
        {-1, -1, -1, -2, -1},
        {3.14, 2.71, 3.14, 3.14, 3.14},
        {5, 5, 5, 5, 4}
    };
    for (const auto& t : tests) {
        cout << findUniq(t) << "\n";
    }
    return 0;
}