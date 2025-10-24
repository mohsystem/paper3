#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

int runCounter(int maxCounter, int numThreads) {
    int currentCounter = 0;
    std::mutex mtx;

    auto worker = [&]() {
        while (true) {
            std::unique_lock<std::mutex> lock(mtx);
            if (currentCounter <= maxCounter) {
                currentCounter++;
                std::cout << "C++ Thread " << std::this_thread::get_id()
                          << " incremented currentCounter to " << currentCounter << std::endl;
            } else {
                break;
            }
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker);
    }
    for (auto &t : threads) {
        t.join();
    }
    return currentCounter;
}

int main() {
    std::cout << "C++ Final: " << runCounter(5, 3) << std::endl;
    std::cout << "C++ Final: " << runCounter(0, 2) << std::endl;
    std::cout << "C++ Final: " << runCounter(10, 5) << std::endl;
    std::cout << "C++ Final: " << runCounter(3, 10) << std::endl;
    std::cout << "C++ Final: " << runCounter(15, 4) << std::endl;
    return 0;
}