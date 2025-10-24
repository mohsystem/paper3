#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>

class FooBar {
private:
    int n;
    std::mutex mtx;
    std::condition_variable cv;
    bool fooTurn;

public:
    FooBar(int n) {
        this->n = n;
        this->fooTurn = true;
    }

    void foo(std::function<void()> printFoo) {
        for (int i = 0; i < n; i++) {
            std::unique_lock<std::mutex> lock(mtx);
            // Wait until it's foo's turn
            cv.wait(lock, [this]{ return fooTurn; });
            printFoo();
            fooTurn = false;
            // Notify the other thread
            cv.notify_one();
        }
    }

    void bar(std::function<void()> printBar) {
        for (int i = 0; i < n; i++) {
            std::unique_lock<std::mutex> lock(mtx);
            // Wait until it's bar's turn
            cv.wait(lock, [this]{ return !fooTurn; });
            printBar();
            fooTurn = true;
            // Notify the other thread
            cv.notify_one();
        }
    }
};

void runTest(int n) {
    if (n <= 0) return;
    FooBar foobar(n);
    auto printFoo = [](){ std::cout << "foo"; };
    auto printBar = [](){ std::cout << "bar"; };

    std::thread threadA(&FooBar::foo, &foobar, printFoo);
    std::thread threadB(&FooBar::bar, &foobar, printBar);

    threadA.join();
    threadB.join();
}

int main() {
    std::vector<int> testCases = {1, 2, 5, 10, 100};
    for (int n : testCases) {
        runTest(n);
        std::cout << std::endl;
    }
    return 0;
}