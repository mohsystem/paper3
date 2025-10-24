#include <iostream>
#include <thread>
#include <functional>
#include <semaphore> // For std::counting_semaphore (C++20)
#include <vector>

class Task188 {
private:
    int n;
    // fooSem starts at 1 (allowed), barSem at 0 (blocked).
    std::counting_semaphore<1> fooSem;
    std::counting_semaphore<1> barSem;

public:
    Task188(int n) : n(n), fooSem(1), barSem(0) {}

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

void run_test(int n) {
    std::cout << "Running test for n = " << n << std::endl;
    if (n <= 0) {
        std::cout << "Output: \"\"" << std::endl;
        std::cout << "--------------------" << std::endl;
        return;
    }

    Task188 fooBar(n);
    auto printFoo = []() { std::cout << "foo"; };
    auto printBar = []() { std::cout << "bar"; };
    
    std::cout << "Output: \"";
    // Pass member function pointers to std::thread
    std::thread threadA(&Task188::foo, &fooBar, printFoo);
    std::thread threadB(&Task188::bar, &fooBar, printBar);

    threadA.join();
    threadB.join();
    
    std::cout << "\"" << std::endl;
    std::cout << "--------------------" << std::endl;
}

int main() {
    std::vector<int> testCases = {1, 2, 5, 10, 0};
    for (int n : testCases) {
        run_test(n);
    }
    return 0;
}