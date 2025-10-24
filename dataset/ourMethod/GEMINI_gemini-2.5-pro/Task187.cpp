#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

// Custom counting semaphore for compatibility before C++20
class Semaphore {
public:
    Semaphore(int count = 0) : count_(count) {}

    void release(int n = 1) {
        std::unique_lock<std::mutex> lock(mutex_);
        count_ += n;
        if (n == 1) {
            cv_.notify_one();
        } else {
            cv_.notify_all();
        }
    }

    void acquire() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return count_ > 0; });
        count_--;
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    int count_;
};

class H2O {
public:
    H2O() : hSem(2), oSem(1) {}

    void hydrogen(std::function<void()> releaseHydrogen) {
        hSem.acquire();
        barrier_wait_and_reset();
        releaseHydrogen();
    }

    void oxygen(std::function<void()> releaseOxygen) {
        oSem.acquire();
        barrier_wait_and_reset();
        releaseOxygen();
    }

private:
    Semaphore hSem;
    Semaphore oSem;
    
    std::mutex barrierMutex;
    std::condition_variable barrierCv;
    int barrierCount = 0;
    int generation = 0;

    void barrier_wait_and_reset() {
        std::unique_lock<std::mutex> lock(barrierMutex);
        int my_gen = generation;
        barrierCount++;
        if (barrierCount == 3) {
            generation++;
            barrierCount = 0;
            // Barrier action: reset semaphores
            hSem.release(2);
            oSem.release(1);
            barrierCv.notify_all();
        } else {
            barrierCv.wait(lock, [this, my_gen] { return my_gen != generation; });
        }
    }
};

void run_simulation(const std::string& water) {
    H2O h2o;
    std::string result;
    std::mutex result_mutex;

    auto releaseHydrogen = [&]() {
        std::lock_guard<std::mutex> lock(result_mutex);
        result += 'H';
    };
    auto releaseOxygen = [&]() {
        std::lock_guard<std::mutex> lock(result_mutex);
        result += 'O';
    };

    std::vector<std::thread> threads;
    for (char atom : water) {
        if (atom == 'H') {
            threads.emplace_back(&H2O::hydrogen, &h2o, releaseHydrogen);
        } else { // 'O'
            threads.emplace_back(&H2O::oxygen, &h2o, releaseOxygen);
        }
    }

    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Input: " << water << std::endl;
    std::cout << "Output: " << result << std::endl;
    std::cout << std::endl;
}

int main() {
    std::vector<std::string> testCases = {"HOH", "OOHHHH", "HHO", "HHHHHHOOO", "OHHOHH"};
    for (const auto& testCase : testCases) {
        run_simulation(testCase);
    }
    return 0;
}