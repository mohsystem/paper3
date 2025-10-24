
#include <iostream>
#include <thread>
#include <semaphore.h>
#include <functional>

class Task188 {
private:
    int n;
    sem_t foo_sem;
    sem_t bar_sem;

public:
    Task188(int n) : n(n) {
        sem_init(&foo_sem, 0, 1);
        sem_init(&bar_sem, 0, 0);
    }

    ~Task188() {
        sem_destroy(&foo_sem);
        sem_destroy(&bar_sem);
    }

    void foo(std::function<void()> printFoo) {
        for (int i = 0; i < n; i++) {
            sem_wait(&foo_sem);
            printFoo();
            sem_post(&bar_sem);
        }
    }

    void bar(std::function<void()> printBar) {
        for (int i = 0; i < n; i++) {
            sem_wait(&bar_sem);
            printBar();
            sem_post(&foo_sem);
        }
    }
};

void testFooBar(int n) {
    Task188 foobar(n);
    
    auto printFoo = []() { std::cout << "foo"; };
    auto printBar = []() { std::cout << "bar"; };
    
    std::thread threadA([&foobar, printFoo]() {
        foobar.foo(printFoo);
    });
    
    std::thread threadB([&foobar, printBar]() {
        foobar.bar(printBar);
    });
    
    threadA.join();
    threadB.join();
    
    std::cout << std::endl;
}

int main() {
    // Test case 1: n = 1
    std::cout << "Test case 1: n = 1" << std::endl;
    testFooBar(1);
    
    // Test case 2: n = 2
    std::cout << "Test case 2: n = 2" << std::endl;
    testFooBar(2);
    
    // Test case 3: n = 5
    std::cout << "Test case 3: n = 5" << std::endl;
    testFooBar(5);
    
    // Test case 4: n = 10
    std::cout << "Test case 4: n = 10" << std::endl;
    testFooBar(10);
    
    // Test case 5: n = 3
    std::cout << "Test case 5: n = 3" << std::endl;
    testFooBar(3);
    
    return 0;
}
