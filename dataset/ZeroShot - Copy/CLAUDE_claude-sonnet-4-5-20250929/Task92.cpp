
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
    Task92() : currentCounter(0), maxCounter(10) {}

    void setMaxCounter(int max) {
        maxCounter = max;
    }

    void resetCounter() {
        currentCounter = 0;
    }

    void accessSharedResource(const std::string& threadName) {
        if (currentCounter <= maxCounter) {
            std::lock_guard<std::mutex> lock(mtx);
            if (currentCounter <= maxCounter) {
                currentCounter++;
                std::cout << threadName << " is accessing currentCounter: " << currentCounter << std::endl;
            }
        }
    }
};

void runTest(int testNum, int maxCounter, int numThreads, int iterations) {
    std::cout << "\\nTest Case " << testNum << ":" << std::endl;
    Task92 task;
    task.setMaxCounter(maxCounter);
    task.resetCounter();

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; i++) {
        std::string threadName = "Thread-" + std::to_string(i + 1);
        threads.push_back(std::thread([&task, threadName, iterations]() {
            for (int j = 0; j < iterations; j++) {
                task.accessSharedResource(threadName);
            }
        }));
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

int main() {
    // Test case 1: 3 threads with maxCounter = 5
    runTest(1, 5, 3, 3);

    // Test case 2: 2 threads with maxCounter = 3
    runTest(2, 3, 2, 2);

    // Test case 3: 4 threads with maxCounter = 8
    runTest(3, 8, 4, 3);

    // Test case 4: 5 threads with maxCounter = 10
    runTest(4, 10, 5, 3);

    // Test case 5: 2 threads with maxCounter = 1
    runTest(5, 1, 2, 1);

    return 0;
}
