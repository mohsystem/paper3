#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>
#include <ctime>

namespace fs = std::filesystem;

// A structure to hold the shared work queue and synchronization primitive
struct WorkQueue {
    std::vector<fs::path> file_paths;
    size_t next_index = 0;
    std::mutex mtx;
};

// Function to simulate processing a single file
void process_file(const fs::path& file_path) {
    // In a real application, this would involve reading, parsing, and processing the file.
    // For this example, we'll just print a message and simulate work.
    std::cout << "Thread " << std::this_thread::get_id() << " started processing: " << file_path.string() << std::endl;
    
    // Simulate some work with a random duration
    std::this_thread::sleep_for(std::chrono::milliseconds(100 + (rand() % 150)));

    std::cout << "Thread " << std::this_thread::get_id() << " finished processing: " << file_path.string() << std::endl;
}

// The function executed by each worker thread
void worker_thread(WorkQueue& queue) {
    while (true) {
        fs::path file_to_process;

        // Scope for the lock guard to ensure the mutex is released
        {
            std::lock_guard<std::mutex> lock(queue.mtx);
            if (queue.next_index >= queue.file_paths.size()) {
                // No more work to be done
                return;
            }
            file_to_process = queue.file_paths[queue.next_index];
            queue.next_index++;
        }

        // Process the file outside the lock to allow other threads to get work
        process_file(file_to_process);
    }
}

// Helper to create a clean directory name from the test name
std::string sanitize_for_path(std::string name) {
    std::string invalid_chars = " /\\:*?\"<>|";
    for (char& c : name) {
        if (invalid_chars.find(c) != std::string::npos) {
            c = '_';
        }
    }
    return name;
}

// Function to run a full test simulation
void run_simulation(const std::string& test_name, int num_files, int num_threads) {
    std::cout << "====================================================\n";
    std::cout << "Running Test: " << test_name << "\n";
    std::cout << "====================================================\n";

    const fs::path dir_path = "test_dir_cpp_" + sanitize_for_path(test_name) + "_" + std::to_string(rand());
    
    // 1. Setup: Create directory and dummy files
    try {
        if (fs::exists(dir_path)) {
            fs::remove_all(dir_path);
        }
        fs::create_directory(dir_path);

        for (int i = 0; i < num_files; ++i) {
            fs::path file_path = dir_path / ("file_" + std::to_string(i) + ".txt");
            std::ofstream outfile(file_path);
            if (outfile.is_open()) {
                outfile << "This is test file " << i << " for test '" << test_name << "'.\n";
                outfile << "It contains some sample data.\n";
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error during setup: " << e.what() << std::endl;
        return;
    }

    // 2. Populate Work Queue
    WorkQueue queue;
    try {
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (entry.is_regular_file()) {
                queue.file_paths.push_back(entry.path());
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error populating queue: " << e.what() << std::endl;
        fs::remove_all(dir_path); // Attempt cleanup on failure
        return;
    }

    // 3. Create and launch worker threads
    std::vector<std::thread> threads;
    if (num_threads > 0) {
        threads.reserve(num_threads);
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(worker_thread, std::ref(queue));
        }
    } else {
         std::cerr << "Number of threads must be positive." << std::endl;
    }


    // 4. Wait for all threads to complete
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // 5. Cleanup
    try {
        fs::remove_all(dir_path);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error during cleanup: " << e.what() << std::endl;
    }
    std::cout << "\nTest '" << test_name << "' finished.\n\n";
}

int main() {
    srand(time(nullptr)); // Seed for random numbers

    run_simulation("Test 1: 5 files, 2 threads", 5, 2);
    run_simulation("Test 2: 10 files, 4 threads", 10, 4);
    run_simulation("Test 3: 20 files, 8 threads", 20, 8);
    run_simulation("Test 4: 2 files, 4 threads (more threads than files)", 2, 4);
    run_simulation("Test 5: 15 files, 1 thread (sequential)", 15, 1);

    return 0;
}