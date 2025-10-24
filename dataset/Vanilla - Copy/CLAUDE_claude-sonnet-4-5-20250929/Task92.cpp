
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <chrono>

class Task92 {
private:
    int currentCounter;
    int maxCounter;
    std::mutex mtx;

public:
    Task92(int maxCounter) : currentCounter(0), maxCounter(maxCounter) {}
    
    void accessSharedResource(const std::string& threadName) {
        if (currentCounter <= maxCounter) {
            mtx.lock();
            if (currentCounter <= maxCounter) {
                currentCounter++;
                std::cout << threadName << " is accessing currentCounter: " << currentCounter << std::endl;
            }
            mtx.unlock();
        }
    }
    
    int getCurrentCounter() {
        return currentCounter;
    }
};

int main() {
    // Test case 1: Basic test with 5 threads and max counter 10
    std::cout << "Test Case 1:" << std::endl;
    Task92 test1(10);
    std::vector<std::thread> threads1;
    for (int i = 1; i <= 5; i++) {
        threads1.push_back(std::thread(&Task92::accessSharedResource, &test1, "Thread-" + std::to_string(i)));
    }
    for (auto& t : threads1) {
        t.join();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Test case 2: Multiple threads with smaller max counter
    std::cout << "\\nTest Case 2:" << std::endl;
    Task92 test2(3);
    std::vector<std::thread> threads2;
    for (int i = 1; i <= 5; i++) {
        threads2.push_back(std::thread(&Task92::accessSharedResource, &test2, "Worker-" + std::to_string(i)));
    }
    for (auto& t : threads2) {
        t.join();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Test case 3: Single thread
    std::cout << "\\nTest Case 3:" << std::endl;
    Task92 test3(1);
    std::thread t3(&Task92::accessSharedResource, &test3, "SingleThread");
    t3.join();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Test case 4: Max counter is 0
    std::cout << "\\nTest Case 4:" << std::endl;
    Task92 test4(0);
    std::vector<std::thread> threads4;
    for (int i = 1; i <= 3; i++) {
        threads4.push_back(std::thread(&Task92::accessSharedResource, &test4, "Thread-" + std::to_string(i)));
    }
    for (auto& t : threads4) {
        t.join();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Test case 5: Many threads with moderate max counter
    std::cout << "\\nTest Case 5:" << std::endl;
    Task92 test5(7);
    std::vector<std::thread> threads5;
    for (int i = 1; i <= 10; i++) {
        threads5.push_back(std::thread(&Task92::accessSharedResource, &test5, "Task-" + std::to_string(i)));
    }
    for (auto& t : threads5) {
        t.join();
    }
    
    return 0;
}
