#include <iostream>
#include <vector>
#include <limits>
#include <string>

struct Result {
    bool ok;
    long long value;
    std::string error;
};

static bool safe_add(long long a, long long b, long long& out) {
    if (b > 0 && a > std::numeric_limits<long long>::max() - b) return false;
    if (b < 0 && a < std::numeric_limits<long long>::min() - b) return false;
    out = a + b;
    return true;
}

Result maxSubarraySum(const std::vector<int>& arr) {
    if (arr.empty()) {
        return {false, 0LL, "invalid input: empty array"};
    }
    long long current = static_cast<long long>(arr[0]);
    long long best = current;

    for (size_t i = 1; i < arr.size(); ++i) {
        long long v = static_cast<long long>(arr[i]);
        long long sum = 0;
        if (!safe_add(current, v, sum)) {
            return {false, 0LL, "sum overflow detected"};
        }
        current = (sum >= v) ? sum : v;
        best = (best >= current) ? best : current;
    }
    return {true, best, ""};
}

static void runTest(int id, const std::vector<int>& input, long long expected = 0, bool hasExpected = false) {
    Result r = maxSubarraySum(input);
    std::cout << "Test " << id << ": input=[";
    for (size_t i = 0; i < input.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << input[i];
    }
    std::cout << "]\n";
    if (r.ok) {
        std::cout << "  ok=true, max_sum=" << r.value;
        if (hasExpected) std::cout << ", expected=" << expected;
        std::cout << "\n";
    } else {
        std::cout << "  ok=false, error=" << r.error << "\n";
    }
}

int main() {
    std::vector<int> t1 = {-2, 1, -3, 4, -1, 2, 1, -5, 4}; // 6
    std::vector<int> t2 = {1, 2, 3, 4}; // 10
    std::vector<int> t3 = {-1, -2, -3}; // -1
    std::vector<int> t4 = {5, -9, 6, -2, 3}; // 7
    std::vector<int> t5; // empty -> error

    runTest(1, t1, 6, true);
    runTest(2, t2, 10, true);
    runTest(3, t3, -1, true);
    runTest(4, t4, 7, true);
    runTest(5, t5);
    return 0;
}