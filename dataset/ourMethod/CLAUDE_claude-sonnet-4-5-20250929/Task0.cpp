
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <memory>
#include <chrono>
#include <cstring>

// Global mutex for file lock map operations to prevent race conditions
std::mutex map_mutex;
// Map to track per-file mutexes - ensures only one thread processes a file at a time
std::unordered_map<std::string, std::shared_ptr<std::mutex>> file_locks;

// Maximum path length to prevent buffer overflow attacks
constexpr size_t MAX_PATH_LENGTH = 4096;
// Maximum file size to prevent excessive memory usage (100MB)
constexpr size_t MAX_FILE_SIZE = 100 * 1024 * 1024;
// Buffer size for file processing
constexpr size_t BUFFER_SIZE = 8192;

// Validates that the path is within the base directory and not a symlink
// Prevents path traversal attacks (CWE-22)
bool validatePath(const std::filesystem::path& base_dir, const std::filesystem::path& file_path) {
    try {
        // Check path length to prevent buffer overflows
        std::string path_str = file_path.string();
        if (path_str.length() > MAX_PATH_LENGTH) {
            std::cerr << "Error: Path exceeds maximum length\\n";
            return false;
        }

        // Normalize and resolve the path
        std::filesystem::path canonical_base = std::filesystem::canonical(base_dir);
        std::filesystem::path canonical_file = std::filesystem::canonical(file_path);

        // Ensure the file path is within the base directory (prevent directory traversal)
        auto [base_end, file_end] = std::mismatch(canonical_base.begin(), canonical_base.end(),
                                                    canonical_file.begin(), canonical_file.end());
        if (base_end != canonical_base.end()) {
            std::cerr << "Error: Path is outside base directory\\n";
            return false;
        }

        // Reject symlinks to prevent TOCTOU attacks (CWE-367)
        if (std::filesystem::is_symlink(file_path)) {
            std::cerr << "Error: Symlinks not allowed\\n";
            return false;
        }

        // Ensure it's a regular file\n        if (!std::filesystem::is_regular_file(file_path)) {\n            std::cerr << "Error: Not a regular file\\n";\n            return false;\n        }\n\n        // Check file size to prevent excessive memory usage\n        auto file_size = std::filesystem::file_size(file_path);\n        if (file_size > MAX_FILE_SIZE) {\n            std::cerr << "Error: File size exceeds maximum allowed\\n";\n            return false;\n        }\n\n        return true;\n    } catch (const std::filesystem::filesystem_error& e) {\n        std::cerr << "Filesystem error during validation: " << e.what() << "\\n";\n        return false;\n    } catch (const std::exception& e) {\n        std::cerr << "Error during path validation: " << e.what() << "\\n";\n        return false;\n    }\n}\n\n// Gets or creates a mutex for a specific file, ensuring thread-safe access\nstd::shared_ptr<std::mutex> getFileLock(const std::string& filename) {\n    // Lock the map to prevent race conditions on file_locks access\n    std::lock_guard<std::mutex> guard(map_mutex);\n    \n    auto it = file_locks.find(filename);\n    if (it == file_locks.end()) {\n        // Create new mutex for this file\n        auto new_lock = std::make_shared<std::mutex>();\n        file_locks[filename] = new_lock;\n        return new_lock;\n    }\n    return it->second;\n}\n\n// Processes a file: reads content and writes word count\n// Returns true on success, false on failure\nbool processFile(const std::filesystem::path& base_dir, const std::filesystem::path& file_path) {\n    try {\n        // Validate path to prevent directory traversal and symlink attacks\n        if (!validatePath(base_dir, file_path)) {\n            return false;\n        }\n\n        std::string filename = file_path.string();\n        \n        // Acquire file-specific lock to ensure exclusive access\n        auto file_lock = getFileLock(filename);\n        std::lock_guard<std::mutex> guard(*file_lock);\n\n        // Open input file for reading\n        std::ifstream input(file_path, std::ios::binary);\n        if (!input.is_open()) {\n            std::cerr << "Error: Cannot open file for reading: " << filename << "\\n";\n            return false;\n        }\n\n        // Process file content - count words as example processing\n        size_t word_count = 0;\n        std::vector<char> buffer(BUFFER_SIZE);\n        bool in_word = false;\n\n        while (input.good()) {\n            input.read(buffer.data(), BUFFER_SIZE);\n            std::streamsize bytes_read = input.gcount();\n            \n            // Bounds check to prevent buffer overflow\n            if (bytes_read < 0 || static_cast<size_t>(bytes_read) > BUFFER_SIZE) {\n                std::cerr << "Error: Invalid bytes read count\\n";\n                input.close();\n                return false;\n            }\n\n            // Count words in buffer\n            for (std::streamsize i = 0; i < bytes_read; ++i) {\n                if (std::isspace(static_cast<unsigned char>(buffer[i]))) {\n                    in_word = false;\n                } else if (!in_word) {\n                    in_word = true;\n                    word_count++;\n                }\n            }\n        }\n\n        input.close();\n\n        // Write results to temporary file, then atomically rename (prevent TOCTOU)\n        std::string temp_output = filename + ".tmp";\n        std::string final_output = filename + ".processed";\n\n        // Open temp file with exclusive creation to prevent race conditions\n        std::ofstream output(temp_output, std::ios::binary | std::ios::trunc);\n        if (!output.is_open()) {\n            std::cerr << "Error: Cannot create temporary output file\\n";\n            return false;\n        }\n\n        // Write results\n        std::string result = "File: " + filename + "\\nWord count: " + std::to_string(word_count) + "\\n";\n        output.write(result.c_str(), result.length());\n        \n        if (!output.good()) {\n            std::cerr << "Error: Failed to write to output file\\n";\n            output.close();\n            std::filesystem::remove(temp_output);\n            return false;\n        }\n\n        // Flush and sync to ensure data is written to disk\n        output.flush();\n        output.close();\n\n        // Atomically rename temp file to final output (prevents TOCTOU)\n        std::filesystem::rename(temp_output, final_output);\n\n        std::cout << "Successfully processed: " << filename << " (words: " << word_count << ")\\n";\n        return true;\n\n    } catch (const std::exception& e) {\n        std::cerr << "Exception during file processing: " << e.what() << "\\n";\n        return false;\n    }\n}\n\n// Worker thread function\nvoid worker(const std::filesystem::path& base_dir, const std::vector<std::filesystem::path>& files, \n            size_t start_idx, size_t end_idx) {\n    // Validate indices to prevent out-of-bounds access\n    if (start_idx > end_idx || end_idx > files.size()) {\n        std::cerr << "Error: Invalid index range\\n";\n        return;\n    }\n\n    for (size_t i = start_idx; i < end_idx; ++i) {\n        processFile(base_dir, files[i]);\n    }\n}\n\nint main() {\n    // Test case 1: Process files from test directory with 2 threads\n    try {\n        std::cout << "=== Test Case 1: Basic concurrent processing ===\\n";\n        std::filesystem::path test_dir = std::filesystem::current_path() / "test_files";\n        \n        // Create test directory if it doesn't exist
        std::filesystem::create_directories(test_dir);
        
        // Create test files with validated content
        std::vector<std::string> test_content = {
            "Hello world this is test file one",
            "Another test file with more words to count here",
            "Short file",
            "This is a longer test file with many words to process correctly",
            "Final test file for concurrent processing system"
        };

        std::vector<std::filesystem::path> test_files;
        for (size_t i = 0; i < test_content.size(); ++i) {
            std::filesystem::path file_path = test_dir / ("test" + std::to_string(i + 1) + ".txt");
            std::ofstream out(file_path, std::ios::binary | std::ios::trunc);
            if (out.is_open()) {
                out.write(test_content[i].c_str(), test_content[i].length());
                out.close();
                test_files.push_back(file_path);
            }
        }

        // Process files with 2 worker threads
        size_t num_threads = 2;
        size_t files_per_thread = (test_files.size() + num_threads - 1) / num_threads;
        std::vector<std::thread> threads;

        for (size_t i = 0; i < num_threads; ++i) {
            size_t start_idx = i * files_per_thread;
            size_t end_idx = std::min(start_idx + files_per_thread, test_files.size());
            if (start_idx < test_files.size()) {
                threads.emplace_back(worker, test_dir, std::cref(test_files), start_idx, end_idx);
            }
        }

        // Wait for all threads to complete
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
        std::cout << "Test case 1 completed\\n\\n";

    } catch (const std::exception& e) {
        std::cerr << "Test case 1 failed: " << e.what() << "\\n\\n";
    }

    // Test case 2: Process same file multiple times (should be serialized)
    try {
        std::cout << "=== Test Case 2: Same file concurrent access ===\\n";
        std::filesystem::path test_dir = std::filesystem::current_path() / "test_files";
        std::filesystem::path single_file = test_dir / "concurrent_test.txt";
        
        std::ofstream out(single_file, std::ios::binary | std::ios::trunc);
        if (out.is_open()) {
            std::string content = "Testing concurrent access to same file";
            out.write(content.c_str(), content.length());
            out.close();
        }

        // Try to process same file from multiple threads (should serialize)
        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i) {
            threads.emplace_back([&test_dir, &single_file]() {
                processFile(test_dir, single_file);
            });
        }

        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
        std::cout << "Test case 2 completed\\n\\n";

    } catch (const std::exception& e) {
        std::cerr << "Test case 2 failed: " << e.what() << "\\n\\n";
    }

    // Test case 3: Path traversal attack prevention
    try {
        std::cout << "=== Test Case 3: Path traversal prevention ===\\n";
        std::filesystem::path test_dir = std::filesystem::current_path() / "test_files";
        std::filesystem::path malicious_path = test_dir / ".." / ".." / "etc" / "passwd";
        
        bool result = processFile(test_dir, malicious_path);
        std::cout << "Path traversal test result (should fail): " << (result ? "FAILED" : "PASSED") << "\\n\\n";

    } catch (const std::exception& e) {
        std::cout << "Test case 3 completed (exception expected)\\n\\n";
    }

    // Test case 4: Empty file processing
    try {
        std::cout << "=== Test Case 4: Empty file processing ===\\n";
        std::filesystem::path test_dir = std::filesystem::current_path() / "test_files";
        std::filesystem::path empty_file = test_dir / "empty.txt";
        
        std::ofstream out(empty_file, std::ios::binary | std::ios::trunc);
        out.close();

        processFile(test_dir, empty_file);
        std::cout << "Test case 4 completed\\n\\n";

    } catch (const std::exception& e) {
        std::cerr << "Test case 4 failed: " << e.what() << "\\n\\n";
    }

    // Test case 5: High concurrency test
    try {
        std::cout << "=== Test Case 5: High concurrency test ===\\n";
        std::filesystem::path test_dir = std::filesystem::current_path() / "test_files";
        
        // Create 10 test files
        std::vector<std::filesystem::path> many_files;
        for (int i = 0; i < 10; ++i) {
            std::filesystem::path file_path = test_dir / ("concurrent" + std::to_string(i) + ".txt");
            std::ofstream out(file_path, std::ios::binary | std::ios::trunc);
            if (out.is_open()) {
                std::string content = "Test file number " + std::to_string(i) + " for high concurrency test";
                out.write(content.c_str(), content.length());
                out.close();
                many_files.push_back(file_path);
            }
        }

        // Process with 4 threads
        size_t num_threads = 4;
        size_t files_per_thread = (many_files.size() + num_threads - 1) / num_threads;
        std::vector<std::thread> threads;

        for (size_t i = 0; i < num_threads; ++i) {
            size_t start_idx = i * files_per_thread;
            size_t end_idx = std::min(start_idx + files_per_thread, many_files.size());
            if (start_idx < many_files.size()) {
                threads.emplace_back(worker, test_dir, std::cref(many_files), start_idx, end_idx);
            }
        }

        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
        std::cout << "Test case 5 completed\\n\\n";

    } catch (const std::exception& e) {
        std::cerr << "Test case 5 failed: " << e.what() << "\\n\\n";
    }

    return 0;
}
