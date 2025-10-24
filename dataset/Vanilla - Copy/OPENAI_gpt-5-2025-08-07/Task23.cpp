#include <iostream>
using namespace std;

long long findNb(long long m) {
    if (m < 0) return -1;
    long long n = 0;
    while (m > 0) {
        n++;
        long long cube = n * n * n;
        m -= cube;
    }
    return (m == 0) ? n : -1;
}

int main() {
    long long tests[] = {
        1071225LL,          // 45
        91716553919377LL,   // -1
        4183059834009LL,    // 2022
        0LL,                // 0
        135440716410000LL   // 4824
    };
    for (long long m : tests) {
        cout << findNb(m) << "\n";
    }
    return 0;
}