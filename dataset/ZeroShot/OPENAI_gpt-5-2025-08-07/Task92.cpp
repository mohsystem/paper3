#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int runCounter(int maxCounter, int numThreads) {
    struct Shared {
        std::mutex mtx;
        std::atomic<int> current{0}; // atomic for initial read outside lock
        int max;
        explicit Shared(int m) : max(m) {}
    } shared(maxCounter);

    auto worker = [&shared](int id) {
        for (;;) {
            if (shared.current.load(std::memory_order_relaxed) <= shared.max) {
                std::unique_lock<std::mutex> lk(shared.mtx);
                if (shared.current.load(std::memory_order_relaxed) <= shared.max) {
                    int newVal = ++shared.current; // safe under lock
                    std::cout << "[CPP] Thread-" << id << " accessing currentCounter=" << newVal << std::endl;
                } else {
                    break;
                }
            } else {
                break;
            }
            std::this_thread::yield();
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(numThreads > 0 ? static_cast<size_t>(numThreads) : 0U);
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, i + 1);
    }
    for (auto &t : threads) {
        if (t.joinable()) t.join();
    }
    return shared.current.load(std::memory_order_relaxed);
}

int main() {
    std::vector<std::pair<int,int>> tests = {
        {5, 3},
        {10, 5},
        {0, 2},
        {1, 4},
        {50, 10}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        int result = runCounter(tests[i].first, tests[i].second);
        std::cout << "[CPP] Test " << (i + 1) << " final currentCounter=" << result
                  << " (max=" << tests[i].first << ", threads=" << tests[i].second << ")\n";
    }
    return 0;
}