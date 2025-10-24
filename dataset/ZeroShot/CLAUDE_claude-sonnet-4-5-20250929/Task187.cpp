
#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore.h>
#include <string>
#include <vector>
#include <functional>

class Task187 {
private:
    sem_t hydrogen_sem;
    sem_t oxygen_sem;
    std::mutex mtx;
    int hydrogen_count;
    std::string result;

public:
    Task187() {
        sem_init(&hydrogen_sem, 0, 2);
        sem_init(&oxygen_sem, 0, 0);
        hydrogen_count = 0;
        result = "";
    }

    ~Task187() {
        sem_destroy(&hydrogen_sem);
        sem_destroy(&oxygen_sem);
    }

    void hydrogen(std::function<void()> releaseHydrogen) {
        sem_wait(&hydrogen_sem);
        mtx.lock();
        releaseHydrogen();
        hydrogen_count++;
        if (hydrogen_count == 2) {
            sem_post(&oxygen_sem);
        }
        mtx.unlock();
    }

    void oxygen(std::function<void()> releaseOxygen) {
        sem_wait(&oxygen_sem);
        mtx.lock();
        releaseOxygen();
        hydrogen_count = 0;
        sem_post(&hydrogen_sem);
        sem_post(&hydrogen_sem);
        mtx.unlock();
    }

    std::string buildWater(const std::string& water) {
        result = "";
        std::vector<std::thread> threads;
        
        for (char c : water) {
            if (c == 'H') {
                threads.emplace_back([this]() {
                    hydrogen([this]() {
                        mtx.lock();
                        result += 'H';
                        mtx.unlock();
                    });
                });
            } else {
                threads.emplace_back([this]() {
                    oxygen([this]() {
                        mtx.lock();
                        result += 'O';
                        mtx.unlock();
                    });
                });
            }
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        return result;
    }
};

int main() {
    std::vector<std::string> testCases = {"HOH", "OOHHHH", "HHOHHO", "HHHHHHOOO", "OHHHOH"};
    
    for (const auto& testCase : testCases) {
        Task187 h2o;
        std::string output = h2o.buildWater(testCase);
        std::cout << "Input: " << testCase << " -> Output: " << output << std::endl;
    }
    
    return 0;
}
