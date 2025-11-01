
#include <iostream>
#include <random>

int generateRandomNumber() {
    std::random_device rd;
    std::mt19937 gen(rd());
    // Generate a random number between 1001 and 100000
    std::uniform_int_distribution<> dis(1001, 100000);
    return dis(gen);
}

void printRandomNumber(int number) {
    if (number > 1000) {
        std::cout << "Generated random number: " << number << std::endl;
    } else {
        std::cout << "Error: Number must be larger than 1000" << std::endl;
    }
}

int main() {
    // Test cases
    for (int i = 0; i < 5; i++) {
        int randomNum = generateRandomNumber();
        printRandomNumber(randomNum);
    }
    return 0;
}
