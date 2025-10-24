#include <bits/stdc++.h>
#include <boost/multiprecision/cpp_int.hpp>
using namespace std;
using boost::multiprecision::cpp_int;

static cpp_int ipow_int(int base, long long exp) {
    cpp_int res = 1;
    for (long long i = 0; i < exp; ++i) res *= base;
    return res;
}

long long digPow(long long n, long long p) {
    string s = to_string(n);
    cpp_int sum = 0;
    long long e = p;
    for (char c : s) {
        int d = c - '0';
        sum += ipow_int(d, e);
        ++e;
    }
    cpp_int k = sum / n;
    if (k * n == sum) {
        if (k > std::numeric_limits<long long>::max()) return -1;
        return static_cast<long long>(k);
    }
    return -1;
}

int main() {
    cout << digPow(89, 1) << "\n";      // 1
    cout << digPow(92, 1) << "\n";      // -1
    cout << digPow(695, 2) << "\n";     // 2
    cout << digPow(46288, 3) << "\n";   // 51
    cout << digPow(135, 1) << "\n";     // 1
    return 0;
}