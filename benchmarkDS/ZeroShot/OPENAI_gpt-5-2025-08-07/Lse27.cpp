#include <iostream>
#include <limits>
#include <stdexcept>

long long checked_add(long long a, long long b) {
    if ((b > 0 && a > std::numeric_limits<long long>::max() - b) ||
        (b < 0 && a < std::numeric_limits<long long>::min() - b)) {
        throw std::overflow_error("Overflow during addition");
    }
    return a + b;
}

long long computeTotalSales(long long m1, long long m2, long long m3) {
    long long total = 0;
    total = checked_add(total, m1);
    total = checked_add(total, m2);
    total = checked_add(total, m3);
    return total;
}

int main() {
    long long totalSales = 0;
    totalSales = checked_add(totalSales, 100);
    totalSales = checked_add(totalSales, 200);
    totalSales = checked_add(totalSales, 300);
    std::cout << totalSales << std::endl;

    try {
        long long ts = 0;
        ts = checked_add(ts, 4567);
        ts = checked_add(ts, 8901);
        ts = checked_add(ts, 2345);
        std::cout << ts << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    try {
        long long ts = 0;
        ts = checked_add(ts, 0);
        ts = checked_add(ts, 999999999);
        ts = checked_add(ts, 1);
        std::cout << ts << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    try {
        long long ts = 0;
        ts = checked_add(ts, 1000000000000LL);
        ts = checked_add(ts, 2000000000000LL);
        ts = checked_add(ts, 3000000000000LL);
        std::cout << ts << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    try {
        long long ts = 0;
        ts = checked_add(ts, 123456789012345LL);
        ts = checked_add(ts, 987654321098765LL);
        ts = checked_add(ts, 111111111111111LL);
        std::cout << ts << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}