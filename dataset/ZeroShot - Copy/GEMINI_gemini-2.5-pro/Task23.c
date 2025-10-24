#include <stdio.h>

/**
 * Finds the number of cubes n such that the sum of the first n cubes
 * equals the given total volume m.
 * The sum is n^3 + (n-1)^3 + ... + 1^3.
 *
 * @param m The total volume of the building.
 * @return The number of cubes n, or -1 if no such n exists.
 */
long long findNb(long long m) {
    long long n = 0;
    long long currentVolume = m;
    while (currentVolume > 0) {
        n++;
        long long cubeVolume = n * n * n;
        currentVolume -= cubeVolume;
    }

    return (currentVolume == 0) ? n : -1;
}

int main() {
    // Test cases
    printf("%lld\n", findNb(1071225LL));
    printf("%lld\n", findNb(91716553919377LL));
    printf("%lld\n", findNb(4183059834009LL));
    printf("%lld\n", findNb(1LL));
    printf("%lld\n", findNb(24723578342962LL));
    return 0;
}