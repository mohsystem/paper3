
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <map>
#include <vector>
#include <functional>
#include <chrono>
#include <condition_variable>
#include <queue>
#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

class Task0 {
private:
    std::string directoryPath;
    std::map<std::string, std::shared_ptr<std::mutex>> fileLocks;
    std::mutex locksMapMutex;
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> taskQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    bool stop;
    int threadPoolSize;
    
    std::shared_ptr<std::mutex> getFileLock(const std::string& fileName) {
        std::lock_guard<std::mutex> lock(locksMapMutex);
        if (fileLocks.find(fileName) == fileLocks.end()) {
            fileLocks[fileName] = std::make_shared<std::mutex>();
        }
        return fileLocks[fileName];
    }
    
    void workerThread() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                cv.wait(lock, [this] { return stop || !taskQueue.empty(); });
                
                if (stop && taskQueue.empty()) {
                    return;
                }
                
                task = std::move(taskQueue.front());
                taskQueue.pop();
            }
            task();
        }
    }
    
public:
    Task0(const std::string& dirPath, int poolSize) 
        : directoryPath(dirPath), stop(false), threadPoolSize(poolSize) {
        for (int i = 0; i < threadPoolSize; ++i) {
            threads.emplace_back(&Task0::workerThread, this);
        }
    }
    
    void processFile(const std::string& fileName, std::function<void(const std::string&)> processor) {
        auto task = [this, fileName, processor]() {
            auto fileLock = getFileLock(fileName);
            
            if (fileLock->try_lock()) {
                std::lock_guard<std::mutex> lock(*fileLock, std::adopt_lock);
                
                std::string filePath = directoryPath + "/" + fileName;
                
                if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
                    processor(filePath);
                    std::cout << "Thread " << std::this_thread::get_id() 
                             << " processed: " << fileName << std::endl;
                } else {
                    std::cout << "File not found: " << fileName << std::endl;
                }
            } else {
                std::cout << "Thread " << std::this_thread::get_id() 
                         << " could not acquire lock for: " << fileName << std::endl;
            }
        };
        
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            taskQueue.push(task);
        }
        cv.notify_one();
    }
    
    void processAllFiles(std::function<void(const std::string&)> processor) {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cout << "Invalid directory: " << directoryPath << std::endl;
            return;
        }
        
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                processFile(entry.path().filename().string(), processor);
            }
        }
    }
    
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stop = true;
        }
        cv.notify_all();
        
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
    
    ~Task0() {
        shutdown();
    }
};

int main() {
    std::string testDir = "test_files_cpp";
    
    fs::create_directory(testDir);
    
    for (int i = 1; i <= 5; ++i) {
        std::ofstream file(testDir + "/file" + std::to_string(i) + ".txt");
        file << "Test content for file " << i << std::endl;
        file.close();
    }
    
    std::cout << "Test Case 1: Process multiple files concurrently" << std::endl;
    {
        Task0 system1(testDir, 3);
        system1.processAllFiles([](const std::string& filePath) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::ifstream file(filePath);
            std::string line;
            std::getline(file, line);
        });
        system1.shutdown();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "\\nTest Case 2: Multiple threads accessing same file" << std::endl;
    {
        Task0 system2(testDir, 5);
        for (int i = 0; i < 3; ++i) {
            system2.processFile("file1.txt", [](const std::string& filePath) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            });
        }
        system2.shutdown();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "\\nTest Case 3: Process with single thread pool" << std::endl;
    {
        Task0 system3(testDir, 1);
        system3.processAllFiles([](const std::string& filePath) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        });
        system3.shutdown();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "\\nTest Case 4: Process non-existent file" << std::endl;
    {
        Task0 system4(testDir, 2);
        system4.processFile("nonexistent.txt", [](const std::string& filePath) {
            std::cout << "Should not reach here" << std::endl;
        });
        system4.shutdown();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    std::cout << "\\nTest Case 5: Process with large thread pool" << std::endl;
    {
        Task0 system5(testDir, 10);
        system5.processAllFiles([](const std::string& filePath) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
        system5.shutdown();
    }
    
    for (const auto& entry : fs::directory_iterator(testDir)) {
        fs::remove(entry.path());
    }
    fs::remove(testDir);
    
    return 0;
}
