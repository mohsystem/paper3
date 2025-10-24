
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>

double getCurrentProcessorTime() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration<double>(duration).count();
}

int main() {
    // Test case 1
    std::cout << "Test case 1:" << std::endl;
    double time1 = getCurrentProcessorTime();
    std::cout << "Current processor time: " << std::fixed << std::setprecision(9) << time1 << " seconds" << std::endl;
    
    // Test case 2
    std::cout << "\\nTest case 2:" << std::endl;
    double time2 = getCurrentProcessorTime();
    std::cout << "Current processor time: " << std::fixed << std::setprecision(9) << time2 << " seconds" << std::endl;
    
    // Test case 3 - with some delay
    std::cout << "\\nTest case 3 (after 1 second delay):" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    double time3 = getCurrentProcessorTime();
    std::cout << "Current processor time: " << std::fixed << std::setprecision(9) << time3 << " seconds" << std::endl;
    
    // Test case 4
    std::cout << "\\nTest case 4:" << std::endl;
    double time4 = getCurrentProcessorTime();
    std::cout << "Current processor time: " << std::fixed << std::setprecision(9) << time4 << " seconds" << std::endl;
    std::cout << "Time difference from test 3: " << std::fixed << std::setprecision(9) << (time4 - time3) << " seconds" << std::endl;
    
    // Test case 5 - with computation
    std::cout << "\\nTest case 5 (after computation):" << std::endl;
    double startTime = getCurrentProcessorTime();
    long long sum = 0;
    for (int i = 0; i < 10000000; i++) {
        sum += i;
    }
    double endTime = getCurrentProcessorTime();
    std::cout << "Start time: " << std::fixed << std::setprecision(9) << startTime << " seconds" << std::endl;
    std::cout << "End time: " << std::fixed << std::setprecision(9) << endTime << " seconds" << std::endl;
    std::cout << "Elapsed time: " << std::fixed << std::setprecision(9) << (endTime - startTime) << " seconds" << std::endl;
    
    return 0;
}
