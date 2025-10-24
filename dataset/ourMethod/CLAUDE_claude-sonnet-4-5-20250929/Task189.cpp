
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <sstream>
#include <string>

class ZeroEvenOdd {
private:
    int n;
    std::mutex mtx;
    std::condition_variable cv;
    int state; // 0: zero should print, 1: odd should print, 2: even should print
    int current; // Current number to be printed (1 to n)
    
public:
    // Initialize with validated input
    ZeroEvenOdd(int n) {
        // Validate input according to constraints: 1 <= n <= 1000
        if (n < 1 || n > 1000) {
            throw std::invalid_argument("n must be between 1 and 1000");
        }
        this->n = n;
        this->state = 0; // Start with zero
        this->current = 1; // First number to print after zero
    }
    
    // Thread A calls this function
    void zero(std::function<void(int)> printNumber) {
        // Validate callback is valid
        if (!printNumber) {
            return;
        }
        
        for (int i = 0; i < n; ++i) {
            std::unique_lock<std::mutex> lock(mtx);
            
            // Wait until it's zero's turn (state == 0)
            cv.wait(lock, [this]() { return state == 0; });
            
            // Critical section: print zero
            printNumber(0);
            
            // Determine next state based on current number
            // If current is odd, next should be odd thread
            // If current is even, next should be even thread
            if (current % 2 == 1) {
                state = 1; // odd's turn\n            } else {\n                state = 2; // even's turn
            }
            
            // Notify all waiting threads
            cv.notify_all();
        }
    }
    
    // Thread B calls this function
    void even(std::function<void(int)> printNumber) {
        // Validate callback is valid
        if (!printNumber) {
            return;
        }
        
        // Only process even numbers from 2 to n
        for (int i = 2; i <= n; i += 2) {
            std::unique_lock<std::mutex> lock(mtx);
            
            // Wait until it's even's turn (state == 2 and current matches)
            cv.wait(lock, [this, i]() { return state == 2 && current == i; });
            
            // Critical section: print even number
            printNumber(i);
            
            // Move to next number
            ++current;
            
            // Check bounds before continuing
            if (current <= n) {
                state = 0; // zero's turn next\n            }\n            \n            // Notify all waiting threads\n            cv.notify_all();\n        }\n    }\n    \n    // Thread C calls this function\n    void odd(std::function<void(int)> printNumber) {\n        // Validate callback is valid\n        if (!printNumber) {\n            return;\n        }\n        \n        // Only process odd numbers from 1 to n\n        for (int i = 1; i <= n; i += 2) {\n            std::unique_lock<std::mutex> lock(mtx);\n            \n            // Wait until it's odd's turn (state == 1 and current matches)\n            cv.wait(lock, [this, i]() { return state == 1 && current == i; });\n            \n            // Critical section: print odd number\n            printNumber(i);\n            \n            // Move to next number\n            ++current;\n            \n            // Check bounds before continuing\n            if (current <= n) {\n                state = 0; // zero's turn next
            }
            
            // Notify all waiting threads
            cv.notify_all();
        }
    }
};

// Test harness
int main() {
    // Test case 1: n = 2, expected output "0102"
    {
        std::stringstream ss;
        ZeroEvenOdd zeo(2);
        
        std::thread t1([&]() { zeo.zero([&](int x) { ss << x; }); });
        std::thread t2([&]() { zeo.even([&](int x) { ss << x; }); });
        std::thread t3([&]() { zeo.odd([&](int x) { ss << x; }); });
        
        t1.join();
        t2.join();
        t3.join();
        
        std::cout << "Test 1 (n=2): " << ss.str() << " (expected: 0102)" << std::endl;
    }
    
    // Test case 2: n = 5, expected output "0102030405"
    {
        std::stringstream ss;
        ZeroEvenOdd zeo(5);
        
        std::thread t1([&]() { zeo.zero([&](int x) { ss << x; }); });
        std::thread t2([&]() { zeo.even([&](int x) { ss << x; }); });
        std::thread t3([&]() { zeo.odd([&](int x) { ss << x; }); });
        
        t1.join();
        t2.join();
        t3.join();
        
        std::cout << "Test 2 (n=5): " << ss.str() << " (expected: 0102030405)" << std::endl;
    }
    
    // Test case 3: n = 1, expected output "01"
    {
        std::stringstream ss;
        ZeroEvenOdd zeo(1);
        
        std::thread t1([&]() { zeo.zero([&](int x) { ss << x; }); });
        std::thread t2([&]() { zeo.even([&](int x) { ss << x; }); });
        std::thread t3([&]() { zeo.odd([&](int x) { ss << x; }); });
        
        t1.join();
        t2.join();
        t3.join();
        
        std::cout << "Test 3 (n=1): " << ss.str() << " (expected: 01)" << std::endl;
    }
    
    // Test case 4: n = 10
    {
        std::stringstream ss;
        ZeroEvenOdd zeo(10);
        
        std::thread t1([&]() { zeo.zero([&](int x) { ss << x; }); });
        std::thread t2([&]() { zeo.even([&](int x) { ss << x; }); });
        std::thread t3([&]() { zeo.odd([&](int x) { ss << x; }); });
        
        t1.join();
        t2.join();
        t3.join();
        
        std::cout << "Test 4 (n=10): " << ss.str() << std::endl;
    }
    
    // Test case 5: n = 3
    {
        std::stringstream ss;
        ZeroEvenOdd zeo(3);
        
        std::thread t1([&]() { zeo.zero([&](int x) { ss << x; }); });
        std::thread t2([&]() { zeo.even([&](int x) { ss << x; }); });
        std::thread t3([&]() { zeo.odd([&](int x) { ss << x; }); });
        
        t1.join();
        t2.join();
        t3.join();
        
        std::cout << "Test 5 (n=3): " << ss.str() << " (expected: 010203)" << std::endl;
    }
    
    return 0;
}
