
#include <iostream>
#include <ctime>
#include <iomanip>

// Function to get current processor time in seconds
// Returns processor time used by the program as a double
// Security: Uses standard library function, no external input, read-only operation
double getCurrentProcessorTime() {
    // clock() returns processor time used by the program
    // CLOCKS_PER_SEC is a constant defined by the implementation
    std::clock_t clockTicks = std::clock();
    
    // Check for clock() failure (returns -1 on error)
    // Prevents use of invalid time value
    if (clockTicks == static_cast<std::clock_t>(-1)) {
        std::cerr << "Error: Unable to retrieve processor time" << std::endl;
        return -1.0;
    }
    
    // Convert clock ticks to seconds
    // Division by CLOCKS_PER_SEC is safe as it's a non-zero constant
    double seconds = static_cast<double>(clockTicks) / CLOCKS_PER_SEC;
    
    return seconds;
}

int main() {
    // Test case 1: Get processor time at program start
    std::cout << "Test 1 - Processor time at start: ";
    double time1 = getCurrentProcessorTime();
    if (time1 >= 0.0) {
        std::cout << std::fixed << std::setprecision(6) << time1 << " seconds" << std::endl;
    }
    
    // Test case 2: Perform some computation and check time
    std::cout << "Test 2 - After small computation: ";
    volatile int sum = 0; // volatile prevents optimization
    for (int i = 0; i < 1000000; ++i) {
        sum += i;
    }
    double time2 = getCurrentProcessorTime();
    if (time2 >= 0.0) {
        std::cout << std::fixed << std::setprecision(6) << time2 << " seconds" << std::endl;
    }
    
    // Test case 3: Multiple reads to show time progression
    std::cout << "Test 3 - Another read: ";
    double time3 = getCurrentProcessorTime();
    if (time3 >= 0.0) {
        std::cout << std::fixed << std::setprecision(6) << time3 << " seconds" << std::endl;
    }
    
    // Test case 4: Check that time is monotonically increasing
    std::cout << "Test 4 - Verify time progression: ";
    double timeA = getCurrentProcessorTime();
    volatile int dummy = 0;
    for (int i = 0; i < 500000; ++i) {
        dummy += i;
    }
    double timeB = getCurrentProcessorTime();
    if (timeA >= 0.0 && timeB >= 0.0) {
        std::cout << "Time increased by " << std::fixed << std::setprecision(6) 
                  << (timeB - timeA) << " seconds" << std::endl;
    }
    
    // Test case 5: Final processor time reading
    std::cout << "Test 5 - Final processor time: ";
    double time5 = getCurrentProcessorTime();
    if (time5 >= 0.0) {
        std::cout << std::fixed << std::setprecision(6) << time5 << " seconds" << std::endl;
    }
    
    return 0;
}
