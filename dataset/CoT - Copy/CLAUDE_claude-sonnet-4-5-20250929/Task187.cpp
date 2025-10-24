
#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore.h>
#include <vector>
#include <string>

class Task187 {
private:
    sem_t hydrogenSemaphore;
    sem_t oxygenSemaphore;
    std::mutex lock;
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
        lock.lock();
        releaseHydrogen();
        hydrogenCount++;
        if (hydrogenCount == 2) {
            sem_post(&oxygenSemaphore);
            hydrogenCount = 0;
        }
        lock.unlock();
    }
    
    void oxygen(std::function<void()> releaseOxygen) {
        sem_wait(&oxygenSemaphore);
        lock.lock();
        releaseOxygen();
        sem_post(&hydrogenSemaphore);
        sem_post(&hydrogenSemaphore);
        lock.unlock();
    }
};

void testCase(const std::string& water) {
    Task187 h2o;
    std::string result;
    std::mutex resultLock;
    
    std::vector<std::thread> threads;
    
    for (char c : water) {
        if (c == 'H') {
            threads.emplace_back([&h2o, &result, &resultLock]() {
                h2o.hydrogen([&result, &resultLock]() {
                    std::lock_guard<std::mutex> guard(resultLock);
                    result += 'H';
                });
            });
        } else {
            threads.emplace_back([&h2o, &result, &resultLock]() {
                h2o.oxygen([&result, &resultLock]() {
                    std::lock_guard<std::mutex> guard(resultLock);
                    result += 'O';
                });
            });
        }
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "Input: " << water << " -> Output: " << result << std::endl;
}

int main() {
    testCase("HOH");
    testCase("OOHHHH");
    testCase("HHHHHHOOO");
    testCase("HHHOHHHOOO");
    testCase("HHOOHH");
    return 0;
}
