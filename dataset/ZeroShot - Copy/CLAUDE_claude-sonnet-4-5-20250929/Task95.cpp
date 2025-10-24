
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
    std::cout << std::fixed << std::setprecision(9);
    std::cout << "Current processor time: " << time1 << " seconds" << std::endl;
    
    // Test case 2
    std::cout << "\\nTest case 2:" << std::endl;
    double time2 = getCurrentProcessorTime();
    std::cout << "Current processor time: " << time2 << " seconds" << std::endl;
    
    // Test case 3 - with some delay
    std::cout << "\\nTest case 3 (after small delay):" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    double time3 = getCurrentProcessorTime();
    std::cout << "Current processor time: " << time3 << " seconds" << std::endl;
    
    // Test case 4
    std::cout << "\\nTest case 4:" << std::endl;
    double time4 = getCurrentProcessorTime();
    std::cout << "Current processor time: " << time4 << " seconds" << std::endl;
    std::cout << "Time elapsed since test case 1: " << (time4 - time1) << " seconds" << std::endl;
    
    // Test case 5
    std::cout << "\\nTest case 5:" << std::endl;
    double time5 = getCurrentProcessorTime();
    std::cout << "Current processor time: " << time5 << " seconds" << std::endl;
    std::cout << "Time elapsed since test case 4: " << (time5 - time4) << " seconds" << std::endl;
    
    return 0;
}
