#include <iostream>

long long ipow(long long base, int exp) {
    long long result = 1;
    long long b = base;
    int e = exp;
    while (e > 0) {
        if (e & 1) result *= b;
        b *= b;
        e >>= 1;
    }
    return result;
}

bool narcissistic(long long n) {
    if (n <= 0) return false;
    int digits = 0;
    long long temp = n;
    while (temp > 0) {
        digits++;
        temp /= 10;
    }
    long long sum = 0;
    temp = n;
    while (temp > 0) {
        int d = static_cast<int>(temp % 10);
        sum += ipow(d, digits);
        temp /= 10;
    }
    return sum == n;
}

int main() {
    std::cout << std::boolalpha;
    long long tests[] = {153, 1652, 7, 370, 9474};
    for (long long t : tests) {
        std::cout << t << " -> " << narcissistic(t) << "\n";
    }
    return 0;
}