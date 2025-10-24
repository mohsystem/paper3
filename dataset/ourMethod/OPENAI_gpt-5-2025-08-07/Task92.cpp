#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <algorithm>

struct SharedState {
    int currentCounter;
    int maxCounter;
    std::mutex mtx;
    SharedState(int start, int maxVal) : currentCounter(start), maxCounter(maxVal) {}
};

static void worker(SharedState* shared, const std::string& name) {
    for (;;) {
        std::lock_guard<std::mutex> lk(shared->mtx);
        if (shared->currentCounter <= shared->maxCounter) {
            shared->currentCounter += 1;
            std::cout << "[" << name << "] incremented currentCounter to "
                      << shared->currentCounter << " (max " << shared->maxCounter << ")\n";
        } else {
            break;
        }
    }
}

int run_counter(int start, int max_value, int thread_count) {
    int threads = thread_count;
    if (threads < 1) return start;
    if (threads > 128) threads = 128;

    SharedState shared(start, max_value);
    std::vector<std::thread> ts;
    ts.reserve(static_cast<size_t>(threads));
    for (int i = 0; i < threads; ++i) {
        std::string name = "Worker-" + std::to_string(i + 1);
        ts.emplace_back(worker, &shared, name);
    }
    for (auto& t : ts) {
        if (t.joinable()) t.join();
    }
    return shared.currentCounter;
}

int main() {
    // 5 test cases
    int result1 = run_counter(0, 5, 3);
    std::cout << "Final counter (test1): " << result1 << "\n";

    int result2 = run_counter(4, 4, 2);
    std::cout << "Final counter (test2): " << result2 << "\n";

    int result3 = run_counter(10, 5, 4);
    std::cout << "Final counter (test3): " << result3 << "\n";

    int result4 = run_counter(0, 50, 8);
    std::cout << "Final counter (test4): " << result4 << "\n";

    int result5 = run_counter(-2, 2, 2);
    std::cout << "Final counter (test5): " << result5 << "\n";

    return 0;
}