#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

// Using a shared atomic index is a simple and efficient way to distribute work
// among threads without needing a complex thread-safe queue for this specific problem.
void file_processor(int thread_id, const std::vector<std::string>& files, std::atomic<size_t>& file_index) {
    while (true) {
        size_t current_index = file_index.fetch_add(1);

        if (current_index >= files.size()) {
            break; // No more files to process
        }

        const std::string& file_name = files[current_index];
        std::cout << "Thread-" << thread_id << " started processing " << file_name << std::endl;
        
        // Simulate file processing time
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        std::cout << "Thread-" << thread_id << " finished processing " << file_name << std::endl;
    }
}

void processFiles(const std::vector<std::string>& files, int num_threads) {
    std::cout << "--- C++ Concurrent File Processing ---" << std::endl;
    if (files.empty()) {
        std::cout << "No files to process." << std::endl;
        std::cout << "All files have been processed." << std::endl << std::endl;
        return;
    }

    std::atomic<size_t> file_index(0);
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(file_processor, i + 1, std::ref(files), std::ref(file_index));
    }

    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }
    
    std::cout << "All files have been processed." << std::endl << std::endl;
}


int main() {
    // Test Case 1
    processFiles({"file1.log", "file2.data", "archive.zip", "image.jpg", "report.docx"}, 3);
    
    // Test Case 2
    processFiles({"document.pdf", "data.csv"}, 2);
    
    // Test Case 3
    processFiles({"single_file.txt"}, 4);
    
    // Test Case 4
    processFiles({"audio.mp3", "video.mp4", "backup.sql", "config.ini", "style.css", "script.js"}, 4);

    // Test Case 5
    processFiles({}, 3); // No files
    
    return 0;
}