#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class FooBar {
private:
    int n;
    std::mutex mtx;
    std::condition_variable cv;
    bool foo_turn;

public:
    FooBar(int n) {
        this->n = n;
        this->foo_turn = true;
    }

    void foo(std::function<void()> printFoo) {
        for (int i = 0; i < n; i++) {
            std::unique_lock<std::mutex> lock(mtx);
            // Wait until it's foo's turn
            cv.wait(lock, [this] { return foo_turn; });
            
        	// printFoo() outputs "foo". Do not change or remove this line.
        	printFoo();
            
            // It's now bar's turn
            foo_turn = false;
            cv.notify_one();
        }
    }

    void bar(std::function<void()> printBar) {
        for (int i = 0; i < n; i++) {
            std::unique_lock<std::mutex> lock(mtx);
            // Wait until it's bar's turn
            cv.wait(lock, [this] { return !foo_turn; });

        	// printBar() outputs "bar". Do not change or remove this line.
        	printBar();

            // It's now foo's turn
            foo_turn = true;
            cv.notify_one();
        }
    }
};

void printFoo() {
    std::cout << "foo";
}

void printBar() {
    std::cout << "bar";
}

void runTest(int n) {
    std::cout << "Test Case (n = " << n << "):" << std::endl;
    FooBar foobar(n);
    
    std::thread threadA(&FooBar::foo, &foobar, printFoo);
    std::thread threadB(&FooBar::bar, &foobar, printBar);
    
    threadA.join();
    threadB.join();
    
    std::cout << std::endl << std::endl;
}

int main() {
    runTest(1);
    runTest(2);
    runTest(5);
    runTest(10);
    runTest(100);
    return 0;
}