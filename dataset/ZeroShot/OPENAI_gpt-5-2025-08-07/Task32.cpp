#include <iostream>
#include <cmath>
#include <vector>

int calculateYears(double principal, double interest, double tax, double desired) {
    if (std::isnan(principal) || std::isnan(interest) || std::isnan(tax) || std::isnan(desired)) return -1;
    if (std::isinf(principal) || std::isinf(interest) || std::isinf(tax) || std::isinf(desired)) return -1;

    if (desired <= principal) return 0;

    double netRate = interest * (1.0 - tax);
    if (netRate <= 0.0) return -1;

    int years = 0;
    const int MAX_YEARS = 10000000;
    double p = principal;

    while (p < desired) {
        double interestEarned = p * interest;
        double afterTaxInterest = interestEarned * (1.0 - tax);
        p += afterTaxInterest;
        years++;
        if (years > MAX_YEARS || std::isnan(p) || std::isinf(p)) return -1;
    }
    return years;
}

int main() {
    std::vector<std::vector<double>> tests = {
        {1000.0, 0.05, 0.18, 1100.0},
        {1000.0, 0.05, 0.18, 1000.0},
        {1000.0, 0.0,  0.18, 1100.0},
        {1200.0, 0.0175, 0.05, 1300.0},
        {1500.0, 0.07, 0.6, 2000.0}
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        int years = calculateYears(tests[i][0], tests[i][1], tests[i][2], tests[i][3]);
        std::cout << "Test " << (i + 1) << ": years = " << years << std::endl;
    }
    return 0;
}