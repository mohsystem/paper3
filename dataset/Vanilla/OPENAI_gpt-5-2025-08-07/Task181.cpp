#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <stdexcept>
#include <functional>

class BankAccount {
private:
    std::mutex mtx;
    bool open_ = false;
    long long balance_ = 0;

    void ensure_open() const {
        if (!open_) throw std::logic_error("Account is closed");
    }

public:
    void open() {
        std::lock_guard<std::mutex> lg(mtx);
        if (open_) throw std::logic_error("Account already open");
        open_ = true;
        balance_ = 0;
    }

    void close() {
        std::lock_guard<std::mutex> lg(mtx);
        if (!open_) throw std::logic_error("Account already closed");
        if (balance_ != 0) throw std::logic_error("Cannot close account with non-zero balance");
        open_ = false;
    }

    long long deposit(long long amount) {
        if (amount < 0) throw std::invalid_argument("Negative deposit");
        std::lock_guard<std::mutex> lg(mtx);
        ensure_open();
        balance_ += amount;
        return balance_;
    }

    long long withdraw(long long amount) {
        if (amount < 0) throw std::invalid_argument("Negative withdraw");
        std::lock_guard<std::mutex> lg(mtx);
        ensure_open();
        if (amount > balance_) throw std::logic_error("Insufficient funds");
        balance_ -= amount;
        return balance_;
    }

    long long get_balance() {
        std::lock_guard<std::mutex> lg(mtx);
        ensure_open();
        return balance_;
    }
};

int main() {
    try {
        // Test 1
        BankAccount a1;
        a1.open();
        a1.deposit(200);
        a1.withdraw(50);
        std::cout << "Test1 balance (expected 150): " << a1.get_balance() << "\n";

        // Test 2: Concurrent deposits
        BankAccount a2;
        a2.open();
        int threads2 = 8;
        int depositsPerThread2 = 10000;
        std::vector<std::thread> workers;
        for (int i = 0; i < threads2; ++i) {
            workers.emplace_back([&a2, depositsPerThread2]() {
                for (int j = 0; j < depositsPerThread2; ++j) {
                    a2.deposit(1);
                }
            });
        }
        for (auto& th : workers) th.join();
        std::cout << "Test2 balance (expected " << (threads2 * depositsPerThread2) << "): " << a2.get_balance() << "\n";

        // Test 3: Over-withdraw should fail gracefully
        BankAccount a3;
        a3.open();
        a3.deposit(50);
        try {
            a3.withdraw(100);
            std::cout << "Test3 unexpected success\n";
        } catch (const std::exception& ex) {
            std::cout << "Test3 over-withdraw failed as expected: " << ex.what() << "\n";
        }

        // Test 4: Close then attempt operation should fail
        BankAccount a4;
        a4.open();
        a4.deposit(75);
        a4.withdraw(75);
        a4.close();
        try {
            a4.deposit(10);
            std::cout << "Test4 unexpected success\n";
        } catch (const std::exception& ex) {
            std::cout << "Test4 operation on closed account failed as expected: " << ex.what() << "\n";
        }

        // Test 5: Mixed concurrent operations
        BankAccount a5;
        a5.open();
        a5.deposit(1000);
        int withdrawThreads = 5;
        int withdrawsPerThread = 100; // total 500
        int depositThreads = 3;
        int depositsPerThread = 100;  // total 300
        std::vector<std::thread> tasks;
        for (int i = 0; i < withdrawThreads; ++i) {
            tasks.emplace_back([&a5, withdrawsPerThread]() {
                for (int j = 0; j < withdrawsPerThread; ++j) {
                    try { a5.withdraw(1); } catch (...) {}
                }
            });
        }
        for (int i = 0; i < depositThreads; ++i) {
            tasks.emplace_back([&a5, depositsPerThread]() {
                for (int j = 0; j < depositsPerThread; ++j) {
                    a5.deposit(1);
                }
            });
        }
        for (auto& th : tasks) th.join();
        std::cout << "Test5 balance (expected 800): " << a5.get_balance() << "\n";
    } catch (const std::exception& ex) {
        std::cout << "Fatal error: " << ex.what() << "\n";
    }
    return 0;
}