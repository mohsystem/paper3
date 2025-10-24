#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <semaphore> // Requires C++20

class H2O {
private:
    // Max constraint is n=20, so 40 H atoms.
    // Semaphore max size can be set generously.
    std::counting_semaphore<> hSem{0};
    std::counting_semaphore<> oSem{0};
    std::mutex mtx;
    int h_atoms_waiting;

public:
    H2O() : h_atoms_waiting(0) {}

    void hydrogen(std::function<void()> releaseHydrogen) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            h_atoms_waiting++;
            if (h_atoms_waiting >= 2) {
                // A pair of H atoms is ready. Wake them up along with an O atom.
                hSem.release(2);
                h_atoms_waiting -= 2;
                oSem.release(1);
            }
        }
        hSem.acquire();
        releaseHydrogen();
    }

    void oxygen(std::function<void()> releaseOxygen) {
        oSem.acquire();
        releaseOxygen();
    }
};

void run_test(const std::string& input) {
    std::cout << "Testing with input: " << input << std::endl;
    std::string output = "";
    std::mutex output_mutex;
    H2O h2o;

    std::vector<std::thread> threads;
    for (char c : input) {
        if (c == 'H') {
            threads.emplace_back([&]() {
                h2o.hydrogen([&]() {
                    std::lock_guard<std::mutex> lock(output_mutex);
                    output += 'H';
                });
            });
        } else { // 'O'
            threads.emplace_back([&]() {
                h2o.oxygen([&]() {
                    std::lock_guard<std::mutex> lock(output_mutex);
                    output += 'O';
                });
            });
        }
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    std::cout << "Output: " << output << std::endl;
    std::cout << "--------------------" << std::endl;
}

int main() {
    std::vector<std::string> testCases = {"HOH", "OOHHHH", "HHHHOO", "HOOHH", "HHHHHHOOO"};
    for (const auto& testCase : testCases) {
        run_test(testCase);
    }
    return 0;
}