#include <bits/stdc++.h>
using namespace std;

long long binaryArrayToInt(const vector<int>& arr) {
    long long acc = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        int bit = arr[i];
        if (bit != 0 && bit != 1) {
            throw invalid_argument("Array must contain only 0 or 1 at index " + to_string(i));
        }
        if (acc > (LLONG_MAX - bit) / 2LL) {
            throw overflow_error("Overflow computing value for provided binary array");
        }
        acc = (acc << 1) + bit;
    }
    return acc;
}

int main() {
    vector<vector<int>> tests = {
        {0, 0, 0, 1}, // 1
        {0, 0, 1, 0}, // 2
        {0, 1, 0, 1}, // 5
        {1, 0, 0, 1}, // 9
        {1, 1, 1, 1}  // 15
    };
    for (const auto& t : tests) {
        try {
            long long result = binaryArrayToInt(t);
            cout << "Testing: [";
            for (size_t i = 0; i < t.size(); ++i) {
                cout << t[i] << (i + 1 < t.size() ? ", " : "");
            }
            cout << "] ==> " << result << "\n";
        } catch (const exception& ex) {
            cout << "Error: " << ex.what() << "\n";
        }
    }
    return 0;
}