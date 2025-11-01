#include <iostream>
#include <random>
#include <limits>

/**
 * Generates a random integer greater than 1000.
 * @return An integer > 1000.
 */
int generateRandomNumber() {
    // Static ensures the random engine is seeded and initialized only once.
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> distrib(1001, std::numeric_limits<int>::max());
    
    return distrib(gen);
}

int main() {
    std::cout << "Generating 5 random numbers greater than 1000:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << generateRandomNumber() << std::endl;
    }
    return 0;
}