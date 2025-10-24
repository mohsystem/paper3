#include <iostream>
#include <ctime>
#include <iomanip>

/**
 * @brief Gets the current processor time in seconds.
 * clock() returns the number of clock ticks elapsed since the program was launched.
 * This value is divided by CLOCKS_PER_SEC to get the time in seconds.
 * 
 * @return The processor time in seconds as a double. Returns -1.0 on failure.
 */
double getProcessorTimeInSeconds() {
    clock_t ticks = std::clock();
    if (ticks == (clock_t)-1) {
        // clock() can fail, although it's rare.
        return -1.0;
    }
    return static_cast<double>(ticks) / CLOCKS_PER_SEC;
}

/**
 * @brief A simple function to consume some CPU time.
 * The 'volatile' keyword suggests to the compiler not to optimize the loop away.
 */
void consumeCpuTime() {
    volatile long long sum = 0;
    for (int i = 0; i < 200000000; ++i) {
        sum += i;
    }
}

int main() {
    std::cout << "Running 5 test cases for getting processor time:" << std::endl;
    std::cout << std::fixed << std::setprecision(6);

    for (int i = 1; i <= 5; ++i) {
        std::cout << "\n--- Test Case " << i << " ---" << std::endl;
        
        double startTime = getProcessorTimeInSeconds();
        if (startTime < 0.0) {
            std::cerr << "Failed to get processor time." << std::endl;
            return 1;
        }
        std::cout << "Initial processor time: " << startTime << " seconds" << std::endl;

        // Consume some CPU time
        consumeCpuTime();

        double endTime = getProcessorTimeInSeconds();
        if (endTime < 0.0) {
            std::cerr << "Failed to get processor time." << std::endl;
            return 1;
        }
        std::cout << "Processor time after work: " << endTime << " seconds" << std::endl;
        std::cout << "Elapsed processor time: " << (endTime - startTime) << " seconds" << std::endl;
    }

    return 0;
}