
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>

class Task92 {
private:
    int currentCounter;
    int maxCounter;
    std::mutex mtx;

public:
    Task92(int maxCount) : currentCounter(0), maxCounter(maxCount) {}

    void accessSharedResource(const std::string& threadName) {
        if (currentCounter <= maxCounter) {
            std::lock_guard<std::mutex> lock(mtx);
            // Double-check after acquiring lock
            if (currentCounter <= maxCounter) {
                currentCounter++;
                std::cout << threadName << " is accessing currentCounter: " << currentCounter << std::endl;
            }
        }
    }

    void resetCounter() {
        std::lock_guard<std::mutex> lock(mtx);
        currentCounter = 0;
    }
};

int main() {
    // Test Case 1: Multiple threads with counter limit 10
    std::cout << "Test Case 1: maxCounter = 10" << std::endl;
    Task92 task1(10);
    std::vector<std::thread> threads1;
    for (int i = 0; i < 5; i++) {
        threads1.push_back(std::thread([&task1, i]() {
            std::string name = "Thread-" + std::to_string(i + 1);
            for (int j = 0; j < 3; j++) {
                task1.accessSharedResource(name);
            }
        }));
    }
    for (auto& t : threads1) {
        t.join();
    }

    // Test Case 2: Counter limit 5
    std::cout << "\\nTest Case 2: maxCounter = 5" << std::endl;
    Task92 task2(5);
    std::vector<std::thread> threads2;
    for (int i = 0; i < 3; i++) {
        threads2.push_back(std::thread([&task2, i]() {
            std::string name = "Thread-" + std::to_string(i + 1);
            for (int j = 0; j < 2; j++) {
                task2.accessSharedResource(name);
            }
        }));
    }
    for (auto& t : threads2) {
        t.join();
    }

    // Test Case 3: Counter limit 0
    std::cout << "\\nTest Case 3: maxCounter = 0" << std::endl;
    Task92 task3(0);
    std::thread t3([&task3]() {
        task3.accessSharedResource("Thread-1");
    });
    t3.join();

    // Test Case 4: Counter limit 20
    std::cout << "\\nTest Case 4: maxCounter = 20" << std::endl;
    Task92 task4(20);
    std::vector<std::thread> threads4;
    for (int i = 0; i < 10; i++) {
        threads4.push_back(std::thread([&task4, i]() {
            std::string name = "Thread-" + std::to_string(i + 1);
            for (int j = 0; j < 2; j++) {
                task4.accessSharedResource(name);
            }
        }));
    }
    for (auto& t : threads4) {
        t.join();
    }

    // Test Case 5: Single thread with counter limit 3
    std::cout << "\\nTest Case 5: Single thread, maxCounter = 3" << std::endl;
    Task92 task5(3);
    std::thread t5([&task5]() {
        for (int i = 0; i < 5; i++) {
            task5.accessSharedResource("Thread-Single");
        }
    });
    t5.join();

    return 0;
}
