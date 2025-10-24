#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

// Shared resources
int currentCounter;
int maxCounter;
std::mutex mtx;

/**
 * The function executed by each thread. It increments the shared counter
 * in a thread-safe manner until it reaches the maxCounter.
 */
void incrementCounter() {
    while (true) {
        // Acquire lock using RAII-style std::lock_guard
        std::lock_guard<std::mutex> guard(mtx);
        
        // Check if the counter is less than the max value
        if (currentCounter < maxCounter) {
            currentCounter++;
            std::cout << "Thread " << std::this_thread::get_id() << " is accessing counter: " << currentCounter << std::endl;
        } else {
            // Max count reached, exit the loop. The lock is released automatically
            // when 'guard' goes out of scope.
            break;
        }
    }
}

/**
 * Sets up and runs a single simulation with a given number of threads and a max counter value.
 * @param numThreads The number of threads to create.
 * @param maxVal The maximum value for the counter.
 */
void runSimulation(int numThreads, int maxVal) {
    std::cout << "--- Starting Test: " << numThreads << " threads, maxCounter = " << maxVal << " ---" << std::endl;

    // Reset shared state for the test
    currentCounter = 0;
    maxCounter = maxVal;

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        // Create and start each thread
        threads.emplace_back(incrementCounter);
    }

    // Wait for all threads to complete their execution
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "--- Test Finished. Final counter value: " << currentCounter << " ---\n" << std::endl;
}

int main() {
    // 5 test cases as required
    runSimulation(2, 10);
    runSimulation(5, 20);
    runSimulation(10, 100);
    runSimulation(3, 5);
    runSimulation(8, 50);
    return 0;
}