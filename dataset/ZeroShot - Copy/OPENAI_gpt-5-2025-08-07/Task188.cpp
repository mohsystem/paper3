#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <vector>

class FooBar {
public:
    explicit FooBar(int n_) : n(n_), turn(0) {}

    void foo() {
        for (int i = 0; i < n; ++i) {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [this] { return turn == 0; });
            out += "foo";
            turn = 1;
            lk.unlock();
            cv.notify_one();
        }
    }

    void bar() {
        for (int i = 0; i < n; ++i) {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [this] { return turn == 1; });
            out += "bar";
            turn = 0;
            lk.unlock();
            cv.notify_one();
        }
    }

    std::string getOutput() const {
        return out;
    }

private:
    int n;
    int turn;
    std::mutex m;
    std::condition_variable cv;
    std::string out;
};

std::string runFooBar(int n) {
    if (n < 1 || n > 1000) {
        return std::string();
    }
    FooBar fb(n);
    std::thread t1(&FooBar::foo, &fb);
    std::thread t2(&FooBar::bar, &fb);
    t1.join();
    t2.join();
    return fb.getOutput();
}

int main() {
    std::vector<int> tests = {1, 2, 3, 5, 10};
    for (int n : tests) {
        std::cout << runFooBar(n) << std::endl;
    }
    return 0;
}