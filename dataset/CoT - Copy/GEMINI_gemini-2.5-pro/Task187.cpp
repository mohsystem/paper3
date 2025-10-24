#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <functional>
#include <semaphore>
#include <barrier>
#include <mutex>

class H2O {
private:
    std::counting_semaphore<2> hSem;
    std::counting_semaphore<1> oSem;
    std::barrier<> barrier;
    
public:
    H2O() : hSem(2), oSem(1), barrier(3, [&]() noexcept {
        hSem.release(2);
        oSem.release(1);
    }) {}

    void hydrogen(std::function<void()> releaseHydrogen) {
        hSem.acquire();
        // releaseHydrogen() outputs "H".
        releaseHydrogen();
        barrier.arrive_and_wait();
    }

    void oxygen(std::function<void()> releaseOxygen) {
        oSem.acquire();
        // releaseOxygen() outputs "O".
        releaseOxygen();
        barrier.arrive_and_wait();
    }
};

class Task187 {
public:
    static std::string runWaterSimulation(const std::string& water) {
        H2O h2o;
        std::string result = "";
        std::mutex resultMutex;
        std::vector<std::thread> threads;

        for (char atom : water) {
            if (atom == 'H') {
                threads.emplace_back(&H2O::hydrogen, &h2o, [&]() {
                    std::lock_guard<std::mutex> lock(resultMutex);
                    result += 'H';
                });
            } else if (atom == 'O') {
                threads.emplace_back(&H2O::oxygen, &h2o, [&]() {
                    std::lock_guard<std::mutex> lock(resultMutex);
                    result += 'O';
                });
            }
        }

        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }

        return result;
    }

    static void main() {
        std::vector<std::string> testCases = {
            "HOH",
            "OOHHHH",
            "HHOHHO",
            "HHHHHHOOO",
            "OHHOHHOHH"
        };

        for (int i = 0; i < testCases.size(); ++i) {
            std::cout << "Test Case " << (i + 1) << ":" << std::endl;
            std::cout << "Input: " << testCases[i] << std::endl;
            std::string output = runWaterSimulation(testCases[i]);
            std::cout << "Output: " << output << std::endl;
            std::cout << "--------------------" << std::endl;
        }
    }
};

int main() {
    Task187::main();
    return 0;
}