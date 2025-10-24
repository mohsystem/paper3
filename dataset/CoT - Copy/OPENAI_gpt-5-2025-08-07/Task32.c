/* Chain-of-Through process:
   1) Problem understanding:
      - Calculate years for principal P to reach desired D with yearly interest I and tax T on interest.
   2) Security requirements:
      - Validate finite, non-negative inputs; avoid infinite loops if no growth.
   3) Secure coding generation:
      - Defensive checks and loop with safeguard.
   4) Code review:
      - Verified finite checks and effective rate logic.
   5) Secure code output:
      - Return -1 for invalid/unreachable scenarios, else years.
*/

#include <stdio.h>
#include <math.h>
#include <float.h>

static int is_finite_double(double x) {
    #if defined(isfinite)
        return isfinite(x);
    #else
        return (!isnan(x) && !isinf(x));
    #endif
}

int calculateYears(double principal, double interest, double tax, double desired) {
    if (!is_finite_double(principal) || !is_finite_double(interest) ||
        !is_finite_double(tax) || !is_finite_double(desired)) {
        return -1;
    }
    if (principal < 0.0 || interest < 0.0 || tax < 0.0 || desired < 0.0) {
        return -1;
    }
    if (desired <= principal) {
        return 0;
    }

    double effectiveRate = interest * (1.0 - tax);
    if (effectiveRate <= 0.0) {
        return -1;
    }

    int years = 0;
    double p = principal;
    const int MAX_YEARS = 100000000;
    while (p < desired) {
        double interestEarned = p * interest;
        double afterTaxInterest = interestEarned * (1.0 - tax);
        p += afterTaxInterest;
        years++;
        if (years > MAX_YEARS) {
            return -1;
        }
    }
    return years;
}

int main(void) {
    printf("%d\n", calculateYears(1000.0, 0.05, 0.18, 1100.0)); // Expected: 3
    printf("%d\n", calculateYears(1000.0, 0.05, 0.18, 1000.0)); // Expected: 0
    printf("%d\n", calculateYears(1000.0, 0.00, 0.10, 1200.0)); // Expected: -1 (unreachable)
    printf("%d\n", calculateYears(1500.0, 0.07, 0.20, 2000.0)); // Expected: 6
    printf("%d\n", calculateYears(500.0, 0.05, 0.00, 525.0));   // Expected: 1
    return 0;
}