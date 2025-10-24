
#include <iostream>
#include <string>
#include <thread>
#include <semaphore.h>
#include <mutex>
#include <vector>
#include <functional>

class Task187 {
private:
    sem_t hydrogenSemaphore;
    sem_t oxygenSemaphore;
    std::mutex mutex;
    int hydrogenCount;

public:
    Task187() {
        sem_init(&hydrogenSemaphore, 0, 2);
        sem_init(&oxygenSemaphore, 0, 0);
        hydrogenCount = 0;
    }

    ~Task187() {
        sem_destroy(&hydrogenSemaphore);
        sem_destroy(&oxygenSemaphore);
    }

    void hydrogen(std::function<void()> releaseHydrogen) {
        sem_wait(&hydrogenSemaphore);
        
        mutex.lock();
        hydrogenCount++;
        if (hydrogenCount == 2) {
            sem_post(&oxygenSemaphore);
        }
        mutex.unlock();
        
        releaseHydrogen();
    }

    void oxygen(std::function<void()> releaseOxygen) {
        sem_wait(&oxygenSemaphore);
        
        releaseOxygen();
        
        mutex.lock();
        hydrogenCount = 0;
        mutex.unlock();
        
        sem_post(&hydrogenSemaphore);
        sem_post(&hydrogenSemaphore);
    }
};

int main() {
    std::vector<std::string> testCases = {"HOH", "OOHHHH", "HHOHHO", "HHHHHHOOO", "OHHHOH"};
    
    for (const auto& water : testCases) {
        std::cout << "Input: " << water << std::endl;
        Task187 h2o;
        std::string result;
        std::mutex resultMutex;
        
        std::vector<std::thread> threads;
        for (char c : water) {
            if (c == 'H') {
                threads.emplace_back([&h2o, &result, &resultMutex]() {
                    h2o.hydrogen([&result, &resultMutex]() {
                        std::lock_guard<std::mutex> lock(resultMutex);
                        result += 'H';
                    });
                });
            } else {
                threads.emplace_back([&h2o, &result, &resultMutex]() {
                    h2o.oxygen([&result, &resultMutex]() {
                        std::lock_guard<std::mutex> lock(resultMutex);
                        result += 'O';
                    });
                });
            }
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        std::cout << "Output: " << result << std::endl << std::endl;
    }
    
    return 0;
}
