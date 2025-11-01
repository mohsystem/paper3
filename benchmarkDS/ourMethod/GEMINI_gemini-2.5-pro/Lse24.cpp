#include <iostream>
#include <random>
#include <stdexcept>

// It's good practice to pass the random number generator by reference
// to avoid re-seeding and re-creation on every function call.
void generateAndCalculateDifference(int min, int max, std::mt19937& gen) {
    if (min > max) {
        // More robust error handling for library functions
        throw std::invalid_argument("Error: min value cannot be greater than max value.");
    }
    
    // Create a uniform distribution for the specified range.
    std::uniform_int_distribution<> distrib(min, max);

    // Generate a random integer between min and max.
    int num1 = distrib(gen);

    // Generate another random integer between min and max.
    int num2 = distrib(gen);

    // Calculate the difference between the two integers.
    int difference = num1 - num2;

    // Print the results.
    std::cout << "First random number: " << num1 << std::endl;
    std::cout << "Second random number: " << num2 << std::endl;
    std::cout << "Difference (" << num1 << " - " << num2 << "): " << difference << std::endl;
}

int main() {
    const int MIN_VAL = 10;
    const int MAX_VAL = 1000;

    // Use std::random_device to seed the Mersenne Twister engine.
    // This provides a non-deterministic random seed if available.
    std::random_device rd;
    std::mt19937 gen(rd());

    std::cout << "Running 5 test cases..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "\nTest Case " << i + 1 << ":" << std::endl;
        try {
            generateAndCalculateDifference(MIN_VAL, MAX_VAL, gen);
        } catch (const std::invalid_argument& e) {
            std::cerr << e.what() << std::endl;
            return 1; // Exit with an error code
        }
    }

    return 0;
}