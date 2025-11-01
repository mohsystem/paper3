
#include <iostream>
#include <cstdlib>
#include <ctime>

int generateRandomNumber() {
    // Generate a random number larger than 1000
    return (rand() % 9000) + 1001;
}

int main() {
    // Seed the random number generator
    srand(time(0));
    
    // Test cases
    for (int i = 1; i <= 5; i++) {
        int randomNum = generateRandomNumber();
        std::cout << "Test case " << i << ": " << randomNum << std::endl;
    }
    
    return 0;
}
