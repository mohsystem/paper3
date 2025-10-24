#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <random>

// A thread-safe queue for distributing file paths to worker threads.
template<typename T>
class SafeQueue {
private:
    std::queue<T> q;
    std::mutex m;

public:
    // Add an item to the queue.
    void push(T value) {
        std::lock_guard<std::mutex> lock(m);
        q.push(std::move(value));
    }

    // Try to get an item from the queue. Returns false if the queue is empty.
    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(m);
        if (q.empty()) {
            return false;
        }
        value = std::move(q.front());
        q.pop();
        return true;
    }
};

// Mutex to protect std::cout from garbled output by multiple threads.
std::mutex cout_mutex;

/**
 * @brief Simulates processing a single file.
 *
 * This function is guaranteed to be called by only one thread for any given file
 * because the file paths are distributed via a thread-safe queue.
 * @param file_path The path to the file to process.
 */
void process_file(const std::filesystem::path& file_path) {
    std::thread::id thread_id = std::this_thread::get_id();

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Thread " << thread_id << " started processing: " << file_path.filename() << std::endl;
    }

    // Simulate work with a random delay
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(50, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(distrib(gen)));

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Thread " << thread_id << " finished processing: " << file_path.filename() << std::endl;
    }
}

/**
 * @brief Worker thread function.
 *
 * Fetches file paths from the queue and processes them until the queue is empty.
 * @param file_queue A reference to the shared thread-safe queue.
 */
void worker(SafeQueue<std::filesystem::path>& file_queue) {
    std::filesystem::path file_path;
    while (file_queue.try_pop(file_path)) {
        process_file(file_path);
    }
}

/**
 * @brief Sets up the environment, runs the simulation, and cleans up.
 *
 * @param directory_path The path to the directory to be created for the test.
 * @param num_files The number of files to create for the test.
 * @param num_threads The number of threads to use for processing.
 */
void run_test(const std::string& directory_path, int num_files, int num_threads) {
    std::cout << "\n--- Running Test ---" << std::endl;
    std::cout << "Directory: " << directory_path << ", Files: " << num_files << ", Threads: " << num_threads << std::endl;
    
    std::filesystem::path dir(directory_path);
    
    try {
        // 1. Setup: Create directory and dummy files
        std::filesystem::create_directories(dir);
        for (int i = 1; i <= num_files; ++i) {
            std::ofstream outfile(dir / ("file_" + std::to_string(i) + ".txt"));
            outfile << "dummy content";
            outfile.close();
        }

        // 2. Create and populate the thread-safe queue
        SafeQueue<std::filesystem::path> file_queue;
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                file_queue.push(entry.path());
            }
        }

        // 3. Create and start worker threads
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(worker, std::ref(file_queue));
        }

        // 4. Wait for all threads to complete
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    // 5. Cleanup: Delete the directory and its contents
    if (std::filesystem::exists(dir)) {
        std::filesystem::remove_all(dir);
        std::cout << "Cleaned up directory: " << directory_path << std::endl;
    }
    std::cout << "--- Test Finished ---" << std::endl;
}

int main() {
    // Main execution with 5 test cases
    try {
        // Test Case 1: More files than threads
        run_test("cpp_test_dir_1", 5, 2);
        // Test Case 2: Equal files and threads
        run_test("cpp_test_dir_2", 4, 4);
        // Test Case 3: More threads than files
        run_test("cpp_test_dir_3", 3, 5);
        // Test Case 4: Single thread
        run_test("cpp_test_dir_4", 5, 1);
        // Test Case 5: Large number of files
        run_test("cpp_test_dir_5", 20, 4);
    } catch (...) {
        std::cerr << "An unknown exception occurred in main." << std::endl;
        return 1;
    }
    return 0;
}