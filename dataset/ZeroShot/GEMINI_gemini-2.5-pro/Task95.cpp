#include <iostream>
#include <ctime>
#include <iomanip>

/**
 * Gets the processor time used by the program since it began, in seconds.
 *
 * @return The processor time in seconds as a double.
 */
double getProcessorTimeInSeconds() {
    // clock() returns the number of clock ticks elapsed since the program was launched.
    // CLOCKS_PER_SEC is a macro defining the number of clock ticks per second.
    // Casting to double before division ensures floating-point arithmetic.
    return static_cast<double>(clock()) / CLOCKS_PER_SEC;
}

/**
 * A simple function to consume some CPU time.
 */
void consumeCpuTime() {
    volatile long sum = 0; // volatile to prevent the compiler from optimizing the loop away
    for (int i = 0; i < 100000000; ++i) {
        sum += i;
    }
}

int main() {
    std::cout << "Running 5 test cases for processor time measurement in C++..." << std::endl;
    for (int i = 1; i <= 5; ++i) {
        double startTime = getProcessorTimeInSeconds();
        std::cout << "Test Case " << i << " (start): " << std::fixed << std::setprecision(6) << startTime << " seconds" << std::endl;

        // Consume some CPU time to see a change
        consumeCpuTime();

        double endTime = getProcessorTimeInSeconds();
        std::cout << "Test Case " << i << " (end):   " << std::fixed << std::setprecision(6) << endTime << " seconds" << std::endl;
        std::cout << "-------------------------------------" << std::endl;
    }
    return 0;
}