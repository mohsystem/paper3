// This code requires C++20 for std::barrier and std::counting_semaphore.
// Compile with: g++ -std=c++20 -pthread your_file_name.cpp -o your_executable
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <functional>
#include <semaphore>
#include <barrier>
#include <mutex>

class H2O {
private:
    std::counting_semaphore<> hSem;
    std::counting_semaphore<> oSem;
    std::barrier<> barrier;

public:
    H2O() : hSem(2), oSem(1), barrier(3, [this]() noexcept {
        // This completion function is called by the last thread to arrive at the barrier.
        // It resets the semaphores for the next molecule.
        hSem.release(2);
        oSem.release(1);
    }) {}

    void hydrogen(std::function<void()> releaseHydrogen) {
        hSem.acquire();
        barrier.arrive_and_wait();
        // releaseHydrogen() outputs "H". Do not change or remove this line.
        releaseHydrogen();
    }

    void oxygen(std::function<void()> releaseOxygen) {
        oSem.acquire();
        barrier.arrive_and_wait();
        // releaseOxygen() outputs "O". Do not change or remove this line.
        releaseOxygen();
    }
};

class Task187 {
public:
    static void runTest(const std::string& input) {
        H2O h2o;
        std::string result = "";
        std::mutex mtx;
        std::vector<std::thread> threads;

        for (char c : input) {
            if (c == 'H') {
                threads.emplace_back(&H2O::hydrogen, &h2o, [&]() {
                    std::lock_guard<std::mutex> lock(mtx);
                    result += 'H';
                });
            } else if (c == 'O') {
                threads.emplace_back(&H2O::oxygen, &h2o, [&]() {
                    std::lock_guard<std::mutex> lock(mtx);
                    result += 'O';
                });
            }
        }

        for (auto& t : threads) {
            t.join();
        }
        
        std::cout << "Input: " << input << std::endl;
        std::cout << "Output: " << result << std::endl;
        std::cout << "---" << std::endl;
    }
};

int main() {
    std::vector<std::string> testCases = {
        "HOH",
        "OOHHHH",
        "HHHHHHOOO",
        "HOHOHH",
        "OOOHHHHHH"
    };

    for (const auto& testCase : testCases) {
        Task187::runTest(testCase);
    }

    return 0;
}