
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class Task188 {
private:
    int n;
    std::mutex mtx;
    std::condition_variable cv;
    bool fooTurn;

public:
    Task188(int n) : n(n), fooTurn(true) {}

    void foo(std::function<void()> printFoo) {
        for (int i = 0; i < n; i++) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]() { return fooTurn; });
            printFoo();
            fooTurn = false;
            cv.notify_one();
        }
    }

    void bar(std::function<void()> printBar) {
        for (int i = 0; i < n; i++) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]() { return !fooTurn; });
            printBar();
            fooTurn = true;
            cv.notify_one();
        }
    }
};

void testFooBar(int n) {
    Task188 fooBar(n);
    
    auto printFoo = []() { std::cout << "foo"; };
    auto printBar = []() { std::cout << "bar"; };
    
    std::thread threadA([&fooBar, printFoo]() {
        fooBar.foo(printFoo);
    });
    
    std::thread threadB([&fooBar, printBar]() {
        fooBar.bar(printBar);
    });
    
    threadA.join();
    threadB.join();
    std::cout << std::endl;
}

int main() {
    std::cout << "Test case 1:" << std::endl;
    testFooBar(1);
    
    std::cout << "\\nTest case 2:" << std::endl;
    testFooBar(2);
    
    std::cout << "\\nTest case 3:" << std::endl;
    testFooBar(5);
    
    std::cout << "\\nTest case 4:" << std::endl;
    testFooBar(10);
    
    std::cout << "\\nTest case 5:" << std::endl;
    testFooBar(3);
    
    return 0;
}
