#include <iostream>
#include <limits>

static long long safe_add_ll(long long a, long long b) {
    if ((b > 0 && a > (std::numeric_limits<long long>::max() - b)) ||
        (b < 0 && a < (std::numeric_limits<long long>::min() - b))) {
        std::cerr << "error\n";
        std::exit(1);
    }
    return a + b;
}

long long computeTotalSales(long long month1, long long month2, long long month3) {
    long long tmp = safe_add_ll(month1, month2);
    return safe_add_ll(tmp, month3);
}

int main() {
    long long totalSales = 0LL;
    totalSales = safe_add_ll(totalSales, 100LL);
    totalSales = safe_add_ll(totalSales, 200LL);
    totalSales = safe_add_ll(totalSales, 300LL);
    std::cout << "Total Sales: " << totalSales << "\n";

    {
        long long totalSales2 = 0LL;
        totalSales2 = safe_add_ll(totalSales2, 0LL);
        totalSales2 = safe_add_ll(totalSales2, 0LL);
        totalSales2 = safe_add_ll(totalSales2, 0LL);
        std::cout << "Total Sales: " << totalSales2 << "\n";
    }

    {
        long long totalSales3 = 0LL;
        totalSales3 = safe_add_ll(totalSales3, 999LL);
        totalSales3 = safe_add_ll(totalSales3, 1LL);
        totalSales3 = safe_add_ll(totalSales3, 0LL);
        std::cout << "Total Sales: " << totalSales3 << "\n";
    }

    {
        long long totalSales4 = 0LL;
        totalSales4 = safe_add_ll(totalSales4, 12345LL);
        totalSales4 = safe_add_ll(totalSales4, 67890LL);
        totalSales4 = safe_add_ll(totalSales4, 111LL);
        std::cout << "Total Sales: " << totalSales4 << "\n";
    }

    {
        long long totalSales5 = 0LL;
        totalSales5 = safe_add_ll(totalSales5, -50LL);
        totalSales5 = safe_add_ll(totalSales5, 100LL);
        totalSales5 = safe_add_ll(totalSales5, 200LL);
        std::cout << "Total Sales: " << totalSales5 << "\n";
    }

    return 0;
}