#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <string>

class Task92 {
public:
    static int run_counter(int maxCounter, int threadCount) {
        if (threadCount <= 0) return 0;
        SharedCounter shared(0, maxCounter);
        std::vector<std::thread> threads;
        threads.reserve(static_cast<size_t>(threadCount));

        for (int i = 0; i < threadCount; ++i) {
            std::string name = "T-" + std::to_string(i);
            threads.emplace_back([&shared, name]() {
                for (;;) {
                    if (!shared.increment_if_allowed(name)) {
                        break;
                    }
                    std::this_thread::yield();
                }
            });
        }
        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }
        return shared.get_current();
    }

private:
    class SharedCounter {
    public:
        SharedCounter(int start, int max) : currentCounter(start), maxCounter(max) {}

        bool increment_if_allowed(const std::string& threadName) {
            std::lock_guard<std::mutex> lg(mtx);
            if (currentCounter <= maxCounter) {
                ++currentCounter;
                std::cout << "Thread " << threadName << " incremented currentCounter to " << currentCounter << std::endl;
                return true;
            }
            return false;
        }

        int get_current() const {
            return currentCounter;
        }

    private:
        int currentCounter;
        int maxCounter;
        mutable std::mutex mtx;
    };
};

int main() {
    std::cout << "Final count (max=5, threads=3): " << Task92::run_counter(5, 3) << std::endl;
    std::cout << "Final count (max=8, threads=4): " << Task92::run_counter(8, 4) << std::endl;
    std::cout << "Final count (max=0, threads=2): " << Task92::run_counter(0, 2) << std::endl;
    std::cout << "Final count (max=1, threads=1): " << Task92::run_counter(1, 1) << std::endl;
    std::cout << "Final count (max=3, threads=5): " << Task92::run_counter(3, 5) << std::endl;
    return 0;
}