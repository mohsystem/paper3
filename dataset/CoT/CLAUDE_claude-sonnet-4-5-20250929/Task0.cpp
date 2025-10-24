
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <queue>
#include <condition_variable>
#include <filesystem>
#include <chrono>
#include <memory>

namespace fs = std::filesystem;

class Task0 {
private:
    std::unordered_map<std::string, std::unique_ptr<std::mutex>> fileLocks;
    std::mutex fileLocksMapMutex;
    std::string baseDirectory;
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> taskQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    bool shutdown;
    int threadPoolSize;

    std::string validateAndNormalizePath(const std::string& path) {
        try {
            fs::path normalizedPath = fs::canonical(fs::absolute(path));
            return normalizedPath.string();
        } catch (const std::exception& e) {
            if (!fs::exists(path)) {
                fs::create_directories(path);
                return fs::absolute(path).string();
            }
            throw std::invalid_argument("Invalid directory path: " + path);
        }
    }

    bool isPathSafe(const std::string& filePath) {
        try {
            fs::path requestedPath = fs::canonical(fs::absolute(filePath));
            fs::path basePath = fs::canonical(baseDirectory);
            std::string reqStr = requestedPath.string();
            std::string baseStr = basePath.string();
            return reqStr.find(baseStr) == 0;
        } catch (const std::exception&) {
            return false;
        }
    }

    std::mutex* getFileLock(const std::string& fullPath) {
        std::lock_guard<std::mutex> lock(fileLocksMapMutex);
        if (fileLocks.find(fullPath) == fileLocks.end()) {
            fileLocks[fullPath] = std::make_unique<std::mutex>();
        }
        return fileLocks[fullPath].get();
    }

    void processFileContent(const std::string& filePath) {
        try {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                std::cerr << "Error: Could not open file " << filePath << std::endl;
                return;
            }

            std::cout << "Thread " << std::this_thread::get_id() 
                     << " processing: " << filePath << std::endl;

            std::string line;
            int lineCount = 0;
            while (std::getline(file, line)) {
                lineCount++;
            }

            std::cout << "Thread " << std::this_thread::get_id() 
                     << " completed: " << filePath << " (" << lineCount << " lines)" << std::endl;

            file.close();
        } catch (const std::exception& e) {
            std::cerr << "Error processing file " << filePath << ": " << e.what() << std::endl;
        }
    }

    void workerThread() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                cv.wait(lock, [this] { return shutdown || !taskQueue.empty(); });

                if (shutdown && taskQueue.empty()) {
                    return;
                }

                task = std::move(taskQueue.front());
                taskQueue.pop();
            }
            task();
        }
    }

public:
    Task0(const std::string& baseDir, int poolSize) 
        : baseDirectory(validateAndNormalizePath(baseDir)), 
          shutdown(false), 
          threadPoolSize(poolSize) {
        
        for (int i = 0; i < threadPoolSize; ++i) {
            threads.emplace_back(&Task0::workerThread, this);
        }
    }

    void processFile(const std::string& fileName) {
        auto task = [this, fileName]() {
            std::string fullPath = (fs::path(baseDirectory) / fileName).string();

            if (!isPathSafe(fullPath)) {
                std::cerr << "Security violation: Path traversal attempt detected for " 
                         << fileName << std::endl;
                return;
            }

            std::mutex* lock = getFileLock(fullPath);

            if (lock->try_lock()) {
                try {
                    processFileContent(fullPath);
                } catch (...) {
                    lock->unlock();
                    throw;
                }
                lock->unlock();
            } else {
                std::cerr << "Could not acquire lock for " << fileName << std::endl;
            }
        };

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            taskQueue.push(task);
        }
        cv.notify_one();
    }

    void shutdownPool() {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            shutdown = true;
        }
        cv.notify_all();

        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    ~Task0() {
        shutdownPool();
    }
};

int main() {
    try {
        std::string testDir = fs::temp_directory_path().string() + "/test_concurrent_files";
        fs::create_directories(testDir);

        for (int i = 1; i <= 5; ++i) {
            std::string fileName = testDir + "/test_file_" + std::to_string(i) + ".txt";
            std::ofstream file(fileName);
            for (int j = 0; j < 100; ++j) {
                file << "Test data line " << j << " in file " << i << "\\n";
            }
            file.close();
        }

        Task0 processor(testDir, 3);

        std::cout << "Test Case 1: Processing multiple files concurrently" << std::endl;
        processor.processFile("test_file_1.txt");
        processor.processFile("test_file_2.txt");
        processor.processFile("test_file_3.txt");

        std::cout << "\\nTest Case 2: Multiple threads attempting same file" << std::endl;
        processor.processFile("test_file_1.txt");
        processor.processFile("test_file_1.txt");

        std::cout << "\\nTest Case 3: Processing remaining files" << std::endl;
        processor.processFile("test_file_4.txt");
        processor.processFile("test_file_5.txt");

        std::cout << "\\nTest Case 4: Path traversal attack attempt" << std::endl;
        processor.processFile("../../../etc/passwd");

        std::cout << "\\nTest Case 5: Non-existent file" << std::endl;
        processor.processFile("non_existent_file.txt");

        std::this_thread::sleep_for(std::chrono::seconds(2));

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
