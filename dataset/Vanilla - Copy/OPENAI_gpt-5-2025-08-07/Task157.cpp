#include <iostream>
#include <vector>

long long allocate_and_sum(int n) {
    if (n <= 0) return 0LL;
    long long* arr = new long long[n];
    for (int i = 0; i < n; ++i) {
        arr[i] = static_cast<long long>(i + 1);
    }
    long long sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];
    }
    delete[] arr;
    return sum;
}

int main() {
    int tests[5] = {0, 1, 5, 10, 100000};
    for (int i = 0; i < 5; ++i) {
        int n = tests[i];
        std::cout << "n=" << n << " sum=" << allocate_and_sum(n) << std::endl;
    }
    return 0;
}