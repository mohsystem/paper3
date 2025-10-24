
#include <iostream>
#include <functional>
#include <thread>
#include <semaphore.h>
#include <string>

class Task189 {
private:
    int n;
    sem_t zeroSem;
    sem_t evenSem;
    sem_t oddSem;
    int current;

public:
    Task189(int n) : n(n), current(1) {
        sem_init(&zeroSem, 0, 1);
        sem_init(&evenSem, 0, 0);
        sem_init(&oddSem, 0, 0);
    }

    ~Task189() {
        sem_destroy(&zeroSem);
        sem_destroy(&evenSem);
        sem_destroy(&oddSem);
    }

    void zero(std::function<void(int)> printNumber) {
        for (int i = 0; i < n; i++) {
            sem_wait(&zeroSem);
            printNumber(0);
            if (current % 2 == 1) {
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
            current++;
            sem_post(&zeroSem);
        }
    }

    void odd(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; i += 2) {
            sem_wait(&oddSem);
            printNumber(i);
            current++;
            sem_post(&zeroSem);
        }
    }
};

void testCase(int n, std::string expected) {
    Task189 zeo(n);
    std::string result;

    auto printNumber = [&result](int x) {
        result += std::to_string(x);
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

    std::cout << "Input: n = " << n << std::endl;
    std::cout << "Output: \\"" << result << "\\"" << std::endl;
    std::cout << "Expected: \\"" << expected << "\\"" << std::endl;
    std::cout << "Pass: " << (result == expected ? "true" : "false") << std::endl;
    std::cout << std::endl;
}

int main() {
    // Test case 1
    testCase(2, "0102");
    
    // Test case 2
    testCase(5, "0102030405");
    
    // Test case 3
    testCase(1, "01");
    
    // Test case 4
    testCase(3, "010203");
    
    // Test case 5
    testCase(10, "01020304050607080910");

    return 0;
}
