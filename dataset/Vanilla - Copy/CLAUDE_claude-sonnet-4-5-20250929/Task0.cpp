
#include <iostream>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <memory>

namespace fs = std::filesystem;

class Task0 {
private:
    std::unordered_map<std::string, std::shared_ptr<std::mutex>> fileLocks;
    std::mutex locksMapMutex;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
    
    std::shared_ptr<std::mutex> getLock(const std::string& filePath) {
        std::lock_guard<std::mutex> lock(locksMapMutex);
        if (fileLocks.find(filePath) == fileLocks.end()) {
            fileLocks[filePath] = std::make_shared<std::mutex>();
        }
        return fileLocks[filePath];
    }
    
    void workerThread() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                condition.wait(lock, [this] { return stop || !tasks.empty(); });
                
                if (stop && tasks.empty()) {
                    return;
                }
                
                task = std::move(tasks.front());
                tasks.pop();
            }
            task();
        }
    }
    
public:
    Task0(int threadPoolSize) : stop(false) {
        for (int i = 0; i < threadPoolSize; ++i) {
            workers.emplace_back(&Task0::workerThread, this);
        }
    }
    
    void processFile(const std::string& filePath, std::function<void(const std::string&)> processor) {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            tasks.emplace([this, filePath, processor]() {
                auto lock = getLock(filePath);
                try {
                    std::lock_guard<std::mutex> fileLock(*lock);
                    std::cout << "Thread " << std::this_thread::get_id() 
                             << " acquired lock for: " << filePath << std::endl;
                    processor(filePath);
                    std::cout << "Thread " << std::this_thread::get_id() 
                             << " finished processing: " << filePath << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Error processing file " << filePath << ": " 
                             << e.what() << std::endl;
                }
                std::cout << "Thread " << std::this_thread::get_id() 
                         << " released lock for: " << filePath << std::endl;
            });
        }
        condition.notify_one();
    }
    
    void processDirectory(const std::string& directoryPath, 
                         std::function<void(const std::string&)> processor) {
        try {
            for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
                if (entry.is_regular_file()) {
                    processFile(entry.path().string(), processor);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error reading directory: " << e.what() << std::endl;
        }
    }
    
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        
        for (std::thread& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    ~Task0() {
        shutdown();
    }
};

void createTestDirectory(const std::string& dirName, int numFiles) {
    fs::create_directories(dirName);
    for (int i = 0; i < numFiles; ++i) {
        std::string fileName = dirName + "/file" + std::to_string(i) + ".txt";
        std::ofstream file(fileName);
        file << "Test content for file " << i;
        file.close();
    }
}

void cleanupTestDirectories() {
    for (int i = 1; i <= 5; ++i) {
        std::string dirName = "test_dir" + std::to_string(i);
        if (fs::exists(dirName)) {
            fs::remove_all(dirName);
        }
    }
}

int main() {
    // Test case 1: Process multiple files concurrently
    std::cout << "=== Test Case 1: Multiple Files Processing ===" << std::endl;
    Task0 system1(3);
    std::string testDir1 = "test_dir1";
    createTestDirectory(testDir1, 3);
    
    auto processor1 = [](const std::string& filePath) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Processed: " << filePath << std::endl;
    };
    
    system1.processDirectory(testDir1, processor1);
    system1.shutdown();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Test case 2: Same file accessed by multiple threads
    std::cout << "\\n=== Test Case 2: Same File Multiple Threads ===" << std::endl;
    Task0 system2(3);
    std::string testDir2 = "test_dir2";
    createTestDirectory(testDir2, 1);
    std::string testFile = testDir2 + "/file0.txt";
    
    for (int i = 0; i < 3; ++i) {
        system2.processFile(testFile, processor1);
    }
    system2.shutdown();
    std::this_thread::sleep_for(std::chrono::seconds(4));
    
    // Test case 3: Large number of files
    std::cout << "\\n=== Test Case 3: Large Number of Files ===" << std::endl;
    Task0 system3(5);
    std::string testDir3 = "test_dir3";
    createTestDirectory(testDir3, 10);
    
    auto processor3 = [](const std::string& filePath) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "Quick process: " << filePath << std::endl;
    };
    
    system3.processDirectory(testDir3, processor3);
    system3.shutdown();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // Test case 4: File content modification
    std::cout << "\\n=== Test Case 4: File Content Modification ===" << std::endl;
    Task0 system4(2);
    std::string testDir4 = "test_dir4";
    createTestDirectory(testDir4, 2);
    
    auto processor4 = [](const std::string& filePath) {
        std::ofstream file(filePath, std::ios::app);
        file << "\\nProcessed by thread " << std::this_thread::get_id();
        file.close();
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    };
    
    system4.processDirectory(testDir4, processor4);
    system4.shutdown();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Test case 5: Error handling
    std::cout << "\\n=== Test Case 5: Error Handling ===" << std::endl;
    Task0 system5(2);
    
    auto processor5 = [](const std::string& filePath) {
        if (filePath.find("error") != std::string::npos) {
            throw std::runtime_error("Simulated error");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "Successfully processed: " << filePath << std::endl;
    };
    
    system5.processFile("test_dir5/error_file.txt", processor5);
    system5.processFile("test_dir5/good_file.txt", processor5);
    system5.shutdown();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    cleanupTestDirectories();
    
    return 0;
}
