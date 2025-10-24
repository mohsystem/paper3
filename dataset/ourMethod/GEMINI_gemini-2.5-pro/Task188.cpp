#include <iostream>
#include <thread>
#include <functional>
#include <semaphore>

// Requires C++20 for std::counting_semaphore
class FooBar {
private:
    int n;
    std::counting_semaphore<1> fooSem{1};
    std::counting_semaphore<1> barSem{0};

public:
    FooBar(int n) {
        this->n = n;
    }

    void foo(std::function<void()> printFoo) {
        for (int i = 0; i < n; i++) {
            fooSem.acquire();
            printFoo();
            barSem.release();
        }
    }

    void bar(std::function<void()> printBar) {
        for (int i = 0; i < n; i++) {
            barSem.acquire();
            printBar();
            fooSem.release();
        }
    }
};

void runTest(int n) {
    std::cout << "Test with n = " << n << ":" << std::endl;
    FooBar fooBar(n);

    auto printFoo = []() { std::cout << "foo" << std::flush; };
    auto printBar = []() { std::cout << "bar" << std::flush; };

    std::thread threadA(&FooBar::foo, &fooBar, printFoo);
    std::thread threadB(&FooBar::bar, &fooBar, printBar);

    threadA.join();
    threadB.join();

    std::cout << "\n" << std::endl;
}

int main() {
    runTest(1);
    runTest(2);
    runTest(5);
    runTest(10);
    runTest(0); // Edge case
    return 0;
}