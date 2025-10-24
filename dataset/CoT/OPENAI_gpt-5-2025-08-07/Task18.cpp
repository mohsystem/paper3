#include <iostream>
#include <vector>
#include <stdexcept>
#include <limits>
using namespace std;

// Chain-of-Through Step 1: Problem understanding
// Convert a vector of 0/1 bits (MSB first) to an integer.

// Chain-of-Through Step 2: Security requirements
// - Validate non-binary values.
// - Prevent integer overflow.

// Chain-of-Through Step 3: Secure coding generation
int binaryArrayToInt(const vector<int>& bits) {
    long long acc = 0; // use wider type to detect overflow
    for (int b : bits) {
        if (b != 0 && b != 1) {
            throw invalid_argument("Vector must contain only 0 or 1.");
        }
        acc = (acc << 1) | b;
        if (acc > std::numeric_limits<int>::max()) {
            throw overflow_error("Integer overflow for the given bit array.");
        }
    }
    return static_cast<int>(acc);
}

// Chain-of-Through Step 4 and 5: Review and finalize (no known vulnerabilities)
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
            int result = binaryArrayToInt(t);
            cout << "Testing: [";
            for (size_t i = 0; i < t.size(); ++i) {
                cout << t[i] << (i + 1 < t.size() ? ", " : "");
            }
            cout << "] ==> " << result << "\n";
        } catch (const exception& ex) {
            cerr << "Error: " << ex.what() << "\n";
        }
    }
    return 0;
}