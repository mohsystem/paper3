#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <string>

// A struct to hold the shared data and the mutex
struct SharedResource {
    int currentCounter;
    const int maxCounter;
    std::mutex mtx;

    SharedResource(int max) : currentCounter(0), maxCounter(max) {}
};

void worker(SharedResource& resource, int thread_id) {
    while (true) {
        bool should_break = false;
        
        // std::lock_guard provides a convenient RAII-style mechanism for owning a mutex
        // The lock is acquired on construction and released on destruction
        {
            std::lock_guard<std::mutex> guard(resource.mtx);
            if (resource.currentCounter < resource.maxCounter) {
                resource.currentCounter++;
                std::cout << "Thread-" << thread_id << " is accessing the counter: " << resource.currentCounter << std::endl;
            } else {
                should_break = true;
            }
        } // The lock is automatically released here
        
        if (should_break) {
            break;
        }
    }
}

int main() {
    const int NUM_THREADS = 5;
    const int MAX_COUNT = 50;

    SharedResource sharedResource(MAX_COUNT);
    std::vector<std::thread> threads;

    std::cout << "Starting " << NUM_THREADS << " threads to count up to " << MAX_COUNT << std::endl;

    // Create and start 5 threads (test cases)
    for (int i = 0; i < NUM_THREADS; ++i) {
        // Pass the shared resource by reference to the thread function
        threads.emplace_back(worker, std::ref(sharedResource), i + 1);
    }

    // Wait for all threads to complete
    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    std::cout << "All threads have finished execution." << std::endl;

    return 0;
}