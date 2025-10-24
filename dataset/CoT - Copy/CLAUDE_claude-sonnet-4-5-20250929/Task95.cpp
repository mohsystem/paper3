
#include <iostream>
#include <ctime>
#include <cmath>
#include <string>
#include <iomanip>

double getProcessorTimeInSeconds() {
    return static_cast<double>(std::clock()) / CLOCKS_PER_SEC;
}

void printProcessorTime() {
    double processorTime = getProcessorTimeInSeconds();
    std::cout << "Current processor time: " << std::fixed << std::setprecision(6) 
              << processorTime << " seconds" << std::endl;
}

int main() {
    std::cout << "Test Case 1:" << std::endl;
    printProcessorTime();
    
    std::cout << "\\nTest Case 2:" << std::endl;
    for (int i = 0; i < 1000000; i++) {
        double temp = std::sqrt(i);
    }
    printProcessorTime();
    
    std::cout << "\\nTest Case 3:" << std::endl;
    for (int i = 0; i < 5000000; i++) {
        double temp = std::sin(i);
    }
    printProcessorTime();
    
    std::cout << "\\nTest Case 4:" << std::endl;
    for (volatile int i = 0; i < 100000000; i++) {
        // Busy wait to consume CPU time
    }
    printProcessorTime();
    
    std::cout << "\\nTest Case 5:" << std::endl;
    for (int i = 0; i < 10000000; i++) {
        std::string s = std::to_string(i);
    }
    printProcessorTime();
    
    return 0;
}
