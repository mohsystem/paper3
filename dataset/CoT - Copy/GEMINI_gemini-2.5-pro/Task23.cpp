#include <iostream>

class Task23 {
public:
    static long long findNb(long long m) {
        long long totalVolume = 0;
        long long n = 0;
        while (totalVolume < m) {
            n++;
            // The result of n*n*n can be large, but for this problem's constraints,
            // it will not overflow a long long before totalVolume exceeds m.
            long long cubeVolume = n * n * n;
            totalVolume += cubeVolume;
        }

        if (totalVolume == m) {
            return n;
        } else {
            return -1;
        }
    }
};

int main() {
    std::cout << "CPP Test Cases:" << std::endl;
    // Test Case 1: Example from description
    std::cout << "findNb(1071225) -> Expected: 45, Got: " << Task23::findNb(1071225) << std::endl;
    // Test Case 2: Example from description, no solution
    std::cout << "findNb(91716553919377LL) -> Expected: -1, Got: " << Task23::findNb(91716553919377LL) << std::endl;
    // Test Case 3: A large number with a solution
    std::cout << "findNb(4183059834009LL) -> Expected: 2022, Got: " << Task23::findNb(4183059834009LL) << std::endl;
    // Test Case 4: Smallest possible input with a solution
    std::cout << "findNb(1) -> Expected: 1, Got: " << Task23::findNb(1) << std::endl;
    // Test Case 5: Another large number with a solution
    std::cout << "findNb(135440716410000LL) -> Expected: 4824, Got: " << Task23::findNb(135440716410000LL) << std::endl;
    return 0;
}