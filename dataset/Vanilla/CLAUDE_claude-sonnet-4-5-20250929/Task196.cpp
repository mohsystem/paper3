
#include <iostream>
#include <thread>
#include <functional>
#include <semaphore.h>
#include <string>
#include <sstream>

class Task196 {
private:
    int n;
    sem_t zeroSem;
    sem_t oddSem;
    sem_t evenSem;

public:
    Task196(int n) : n(n) {
        sem_init(&zeroSem, 0, 1);
        sem_init(&oddSem, 0, 0);
        sem_init(&evenSem, 0, 0);
    }

    ~Task196() {
        sem_destroy(&zeroSem);
        sem_destroy(&oddSem);
        sem_destroy(&evenSem);
    }

    void zero(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; i++) {
            sem_wait(&zeroSem);
            printNumber(0);
            if (i % 2 == 1) {
                sem_post(&oddSem);
            } else {
                sem_post(&evenSem);
            }
        }
    }

    void even(std::function<void(int)> printNumber) {
        for (int i = 2; i <= n; i += 2) {
            sem_wait(&evenSem);
            printNumber(i);
            sem_post(&zeroSem);
        }
    }

    void odd(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; i += 2) {
            sem_wait(&oddSem);
            printNumber(i);
            sem_post(&zeroSem);
        }
    }
};

void testCase(int n) {
    Task196 zeo(n);
    std::stringstream result;
    
    auto printNumber = [&result](int x) {
        result << x;
    };
    
    std::thread t1([&zeo, &printNumber]() {
        zeo.zero(printNumber);
    });
    
    std::thread t2([&zeo, &printNumber]() {
        zeo.even(printNumber);
    });
    
    std::thread t3([&zeo, &printNumber]() {
        zeo.odd(printNumber);
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    std::cout << result.str() << std::endl;
}

int main() {
    std::cout << "Test 1 (n=2): ";
    testCase(2);
    
    std::cout << "Test 2 (n=5): ";
    testCase(5);
    
    std::cout << "Test 3 (n=1): ";
    testCase(1);
    
    std::cout << "Test 4 (n=10): ";
    testCase(10);
    
    std::cout << "Test 5 (n=7): ";
    testCase(7);
    
    return 0;
}
