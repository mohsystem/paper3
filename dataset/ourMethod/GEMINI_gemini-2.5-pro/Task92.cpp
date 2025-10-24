#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <sstream>

struct SharedResource {
    int currentCounter = 0;
    const int maxCounter;
    std::mutex mtx;

    SharedResource(int max) : maxCounter(max) {}
};

void worker(SharedResource& resource) {
    while (true) {
        int localValue = -1;
        bool shouldBreak = false;

        {
            // Use lock_guard for RAII-style mutex management
            std::lock_guard<std::mutex> guard(resource.mtx);
            // Check-then-act is performed atomically inside the lock to prevent TOCTOU
            if (resource.currentCounter < resource.maxCounter) {
                resource.currentCounter++;
                localValue = resource.currentCounter;
            } else {
                shouldBreak = true;
            }
        } // Mutex is released here

        if (shouldBreak) {
            break;
        }
        
        if (localValue != -1) {
            // Use a stringstream to build the message to avoid interleaved output
            std::stringstream ss;
            ss << "Thread " << std::this_thread::get_id() 
               << " is accessing counter: " << localValue << "\n";
            std::cout << ss.str();
        }
    }
}

void runTest(int numThreads, int maxCounter) {
    std::cout << "--- Running test with " << numThreads 
              << " threads and max counter " << maxCounter << " ---" << std::endl;
    
    SharedResource resource(maxCounter);
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, std::ref(resource));
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    std::cout << "--- Test finished. Final counter should be " << maxCounter << " ---" << std::endl;
    std::cout << std::endl;
}

int main() {
    // 5 test cases
    runTest(5, 10);
    runTest(10, 20);
    runTest(2, 50);
    runTest(20, 100);
    runTest(8, 8);

    return 0;
}