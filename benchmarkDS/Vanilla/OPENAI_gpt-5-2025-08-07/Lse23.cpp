#include <bits/stdc++.h>
using namespace std;

int runCase(unsigned long long seed) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> dist(1001, 10000);
    int a = dist(rng);
    int b = dist(rng);
    int c = dist(rng);
    int sum = a + b + c;
    cout << "a : " << a << " b : " << b << " c : " << c << "\n";
    cout << "sum: " << sum << " sum2: " << sum << "\n";
    return sum;
}

int main() {
    runCase(1);
    runCase(2);
    runCase(3);
    runCase(4);
    runCase(5);
    return 0;
}