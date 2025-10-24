#include <iostream>
#include <ctime>
#include <iomanip>

/**
 * @brief Gets the processor time used by the program since it started.
 * This is measured in clock ticks, which are converted to seconds.
 * @return The processor time in seconds as a double.
 */
double getProcessorTimeInSeconds() {
    // clock() returns the number of clock ticks elapsed since the program was launched.
    // CLOCKS_PER_SEC is the number of clock ticks per second.
    return static_cast<double>(clock()) / CLOCKS_PER_SEC;
}

/**
 * @brief A simple function to consume some CPU time to make the measurements meaningful.
 */
void doSomeWork() {
    long sum = 0;
    // C++ is faster, so a larger loop is used to see a time difference.
    for (long i = 0; i < 200000000L; ++i) {
        sum += i;
    }
}

int main() {
    std::cout << "Running 5 test cases to measure processor time..." << std::endl;
    
    // Set precision for floating-point output
    std::cout << std::fixed << std::setprecision(6);
    
    // Test Case 1: Initial measurement
    std::cout << "Test Case 1: Start time: " << getProcessorTimeInSeconds() << " seconds" << std::endl;

    // Test Case 2: After some work
    doSomeWork();
    std::cout << "Test Case 2: After first work: " << getProcessorTimeInSeconds() << " seconds" << std::endl;

    // Test Case 3: After more work
    doSomeWork();
    std::cout << "Test Case 3: After second work: " << getProcessorTimeInSeconds() << " seconds" << std::endl;
    
    // Test Case 4: A measurement immediately after the previous one
    std::cout << "Test Case 4: Immediately after: " << getProcessorTimeInSeconds() << " seconds" << std::endl;

    // Test Case 5: After a final round of work
    doSomeWork();
    std::cout << "Test Case 5: After final work: " << getProcessorTimeInSeconds() << " seconds" << std::endl;

    return 0;
}