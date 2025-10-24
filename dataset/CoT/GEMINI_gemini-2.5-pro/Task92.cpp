#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

// Use a class to hold shared data and the lock
class Task92 {
public:
    int currentCounter;
    std::mutex mtx;

    Task92() : currentCounter(0) {}
};

// The function to be executed by each thread
void accessResource(Task92& task, int maxCounter) {
    while (true) {
        // std::lock_guard automatically locks the mutex on creation
        // and unlocks it on destruction (when it goes out of scope)
        std::lock_guard<std::mutex> lock(task.mtx);
        
        if (task.currentCounter < maxCounter) {
            task.currentCounter++;
            std::cout << "Thread " << std::this_thread::get_id() << " is accessing counter: " << task.currentCounter << std::endl;
        } else {
            // Exit the loop if the counter has reached its max
            break;
        }
        // Lock is released here as 'lock' goes out of scope for this iteration
        // Note: It's better to release lock before sleeping
        // A better pattern:
        // { std::lock_guard lock(mtx); ... }
        // std::this_thread::sleep_for(...)
    }
}


void run_test_case(int num_threads, int max_counter) {
    std::cout << "\n--- Test Case: " << num_threads << " threads, max_counter " << max_counter << " ---" << std::endl;
    
    Task92 task;
    std::vector<std::thread> threads;

    // Create and start threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(accessResource, std::ref(task), max_counter);
    }

    // Wait for all threads to complete
    for (auto& th : threads) {
        th.join();
    }

    std::cout << "All threads have finished. Final counter value: " << task.currentCounter << std::endl;
}

int main() {
    // 5 test cases
    run_test_case(5, 20);
    run_test_case(3, 10);
    run_test_case(8, 50);
    run_test_case(2, 5);
    run_test_case(10, 10);
    
    return 0;
}