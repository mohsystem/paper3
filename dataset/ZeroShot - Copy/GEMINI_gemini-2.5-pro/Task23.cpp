#include <iostream>

class Task23 {
public:
    /**
     * Finds the number of cubes n such that the sum of the first n cubes
     * equals the given total volume m.
     * The sum is n^3 + (n-1)^3 + ... + 1^3.
     *
     * @param m The total volume of the building.
     * @return The number of cubes n, or -1 if no such n exists.
     */
    static long long findNb(long long m) {
        long long n = 0;
        long long currentVolume = m;
        while (currentVolume > 0) {
            n++;
            long long cubeVolume = n * n * n;
            currentVolume -= cubeVolume;
        }

        return (currentVolume == 0) ? n : -1;
    }
};

int main() {
    // Test cases
    std::cout << Task23::findNb(1071225LL) << std::endl;
    std::cout << Task23::findNb(91716553919377LL) << std::endl;
    std::cout << Task23::findNb(4183059834009LL) << std::endl;
    std::cout << Task23::findNb(1LL) << std::endl;
    std::cout << Task23::findNb(24723578342962LL) << std::endl;
    return 0;
}