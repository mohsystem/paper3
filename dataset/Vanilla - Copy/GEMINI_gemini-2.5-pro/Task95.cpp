#include <iostream>
#include <ctime>
#include <cmath>
#include <iomanip>
#include <string>

/**
 * @brief Returns the processor time used by the program since its start.
 * 
 * @return The processor time in seconds as a double.
 */
double getProcessorTimeInSeconds() {
    return static_cast<double>(clock()) / CLOCKS_PER_SEC;
}

int main() {
    std::cout << "CPP Processor Time Test Cases:" << std::endl;
    // Set output precision for floating-point numbers
    std::cout << std::fixed << std::setprecision(9);

    // Test Case 1: Initial time
    std::cout << "Test 1: " << getProcessorTimeInSeconds() << " seconds" << std::endl;

    // Do some work to consume CPU time
    for (long i = 0; i < 200000000L; ++i) {
        // busy loop
    }

    // Test Case 2: Time after some work
    std::cout << "Test 2: " << getProcessorTimeInSeconds() << " seconds" << std::endl;

    // Do more intensive work
    double sum = 0;
    for (int i = 0; i < 10000000; ++i) {
        sum += sqrt(static_cast<double>(i));
    }

    // Test Case 3: Time after more work
    std::cout << "Test 3: " << getProcessorTimeInSeconds() << " seconds" << std::endl;

    // Test Case 4: Time immediately after, should be very similar to Test 3
    std::cout << "Test 4: " << getProcessorTimeInSeconds() << " seconds" << std::endl;

    // Do some final work
    std::string s = "";
    for (int i = 0; i < 20000; ++i) {
        s += "a";
    }

    // Test Case 5: Final time
    std::cout << "Test 5: " << getProcessorTimeInSeconds() << " seconds" << std::endl;

    return 0;
}