#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <system_error>

namespace fs = std::filesystem;

// Global constants
const std::string DIR_PATH = "cpp_test_files";
const int NUM_FILES = 5; // Test case: 5 files
const int NUM_THREADS = 3;

// Shared data and synchronization primitive
std::queue<fs::path> g_file_queue;
std::mutex g_queue_mutex;

void file_processor(int thread_id) {
    while (true) {
        fs::path file_path;

        // Scope for the lock to ensure it's released quickly
        {
            std::lock_guard<std::mutex> lock(g_queue_mutex);
            if (g_file_queue.empty()) {
                // No more files to process
                return;
            }
            file_path = g_file_queue.front();
            g_file_queue.pop();
        }

        std::cout << "Thread " << thread_id << " is processing file: " << file_path.filename().string() << std::endl;

        try {
            // Simulate processing time
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Append a message to the file
            std::ofstream file(file_path, std::ios::app);
            if (file.is_open()) {
                file << "\nProcessed by Thread " << thread_id;
                file.close();
            } else {
                std::cerr << "Thread " << thread_id << " failed to open file: " << file_path << std::endl;
            }

            std::cout << "Thread " << thread_id << " finished processing file: " << file_path.filename().string() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Thread " << thread_id << " encountered an error with " << file_path.filename().string() << ": " << e.what() << std::endl;
        }
    }
}

void setup_test_environment() {
    std::cout << "Setting up test environment..." << std::endl;
    try {
        if (fs::exists(DIR_PATH)) {
            fs::remove_all(DIR_PATH);
        }
        fs::create_directory(DIR_PATH);

        for (int i = 1; i <= NUM_FILES; ++i) {
            fs::path file_path = fs::path(DIR_PATH) / ("test_file_" + std::to_string(i) + ".txt");
            std::ofstream file(file_path);
            if (file.is_open()) {
                file << "This is test file " << i << ".";
                file.close();
            }
        }
        std::cout << NUM_FILES << " test files created in " << DIR_PATH << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to set up test environment: " << e.what() << std::endl;
        exit(1);
    }
}

void cleanup_test_environment() {
    std::cout << "\nCleaning up test environment..." << std::endl;
    try {
        if (fs::exists(DIR_PATH)) {
            fs::remove_all(DIR_PATH);
            std::cout << "Test directory and files deleted." << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to clean up test environment: " << e.what() << std::endl;
    }
}

int main() {
    // 1. Setup: Create directory and 5 test files
    setup_test_environment();

    // 2. Populate the shared queue with files to process
    try {
        for (const auto& entry : fs::directory_iterator(DIR_PATH)) {
            if (entry.is_regular_file()) {
                g_file_queue.push(entry.path());
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error reading directory: " << e.what() << std::endl;
        cleanup_test_environment();
        return 1;
    }
    
    std::cout << "\nStarting " << NUM_THREADS << " threads to process " << g_file_queue.size() << " files." << std::endl;

    // 3. Create and start worker threads
    std::vector<std::thread> threads;
    for (int i = 1; i <= NUM_THREADS; ++i) {
        threads.emplace_back(file_processor, i);
    }

    // 4. Wait for all threads to complete
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    std::cout << "\nAll processing tasks are complete." << std::endl;

    // 5. Cleanup: Remove the directory and its contents
    cleanup_test_environment();

    return 0;
}