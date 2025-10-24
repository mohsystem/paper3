#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <stdexcept>

class BankAccount {
private:
    long balance;
    bool isOpen;
    mutable std::mutex mtx;

public:
    BankAccount() : balance(0), isOpen(false) {}

    void open(long initialDeposit) {
        std::lock_guard<std::mutex> lock(mtx);
        if (initialDeposit < 0) {
            throw std::invalid_argument("Initial deposit cannot be negative.");
        }
        balance = initialDeposit;
        isOpen = true;
    }

    void close() {
        std::lock_guard<std::mutex> lock(mtx);
        isOpen = false;
    }

    long getBalance() const {
        std::lock_guard<std::mutex> lock(mtx);
        if (!isOpen) {
            throw std::logic_error("Account is closed.");
        }
        return balance;
    }

    void deposit(long amount) {
        std::lock_guard<std::mutex> lock(mtx);
        if (!isOpen) {
            throw std::logic_error("Account is closed.");
        }
        if (amount <= 0) {
            throw std::invalid_argument("Deposit amount must be positive.");
        }
        balance += amount;
    }

    void withdraw(long amount) {
        std::lock_guard<std::mutex> lock(mtx);
        if (!isOpen) {
            throw std::logic_error("Account is closed.");
        }
        if (amount <= 0) {
            throw std::invalid_argument("Withdrawal amount must be positive.");
        }
        if (balance < amount) {
            throw std::runtime_error("Insufficient funds.");
        }
        balance -= amount;
    }
};

void concurrent_worker(BankAccount& account, bool isDepositor, int transactions) {
    try {
        for (int i = 0; i < transactions; ++i) {
            if (isDepositor) {
                account.deposit(10);
            } else {
                account.withdraw(10);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in thread: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "--- C++ Bank Account Tests ---" << std::endl;

    // Test Case 1: Simple Deposit/Withdraw
    std::cout << "\n--- Test Case 1: Simple Transactions ---" << std::endl;
    try {
        BankAccount acc1;
        acc1.open(100);
        std::cout << "Opened with balance: " << acc1.getBalance() << std::endl;
        acc1.deposit(50);
        std::cout << "After deposit of 50, balance: " << acc1.getBalance() << std::endl;
        acc1.withdraw(30);
        std::cout << "After withdrawal of 30, balance: " << acc1.getBalance() << std::endl;
        if (acc1.getBalance() == 120) {
            std::cout << "Test Case 1 PASSED" << std::endl;
        } else {
            std::cout << "Test Case 1 FAILED" << std::endl;
        }

        // Test Case 2: Insufficient Funds
        std::cout << "\n--- Test Case 2: Insufficient Funds ---" << std::endl;
        try {
            std::cout << "Attempting to withdraw 200 from " << acc1.getBalance() << "..." << std::endl;
            acc1.withdraw(200);
            std::cout << "Test Case 2 FAILED" << std::endl;
        } catch (const std::runtime_error& e) {
            std::cout << "Caught expected exception: " << e.what() << std::endl;
            std::cout << "Test Case 2 PASSED" << std::endl;
        }

        // Test Case 3: Closed Account Operations
        std::cout << "\n--- Test Case 3: Closed Account ---" << std::endl;
        acc1.close();
        std::cout << "Account closed." << std::endl;
        try {
            acc1.getBalance();
            std::cout << "Test Case 3 FAILED (getBalance on closed account)" << std::endl;
        } catch (const std::logic_error& e) {
            std::cout << "Caught expected exception on getBalance: " << e.what() << std::endl;
            std::cout << "Test Case 3 PASSED" << std::endl;
        }
    } catch(const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }

    // Test Case 4: Concurrent Transactions
    std::cout << "\n--- Test Case 4: Concurrent Transactions ---" << std::endl;
    try {
        BankAccount sharedAccount;
        sharedAccount.open(10000);
        int transactions = 1000;
        int num_thread_pairs = 10;
        std::vector<std::thread> threads;

        for (int i = 0; i < num_thread_pairs; ++i) {
            threads.emplace_back(concurrent_worker, std::ref(sharedAccount), true, transactions);
            threads.emplace_back(concurrent_worker, std::ref(sharedAccount), false, transactions);
        }
        for (auto& t : threads) t.join();
        
        long finalBalance = sharedAccount.getBalance();
        std::cout << "Initial Balance: 10000" << std::endl;
        std::cout << "Final Balance after concurrent transactions: " << finalBalance << std::endl;
        if (finalBalance == 10000) {
            std::cout << "Test Case 4 PASSED" << std::endl;
        } else {
            std::cout << "Test Case 4 FAILED" << std::endl;
        }
    } catch(const std::exception& e) {
        std::cerr << "An unexpected error occurred in concurrency test: " << e.what() << std::endl;
    }

    // Test Case 5: Zero/Negative Amount
    std::cout << "\n--- Test Case 5: Invalid Amount ---" << std::endl;
    try {
        BankAccount acc5;
        acc5.open(100);
        try {
            std::cout << "Attempting to deposit -50..." << std::endl;
            acc5.deposit(-50);
            std::cout << "Test Case 5 FAILED (negative deposit)" << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cout << "Caught expected exception: " << e.what() << std::endl;
        }
        try {
            std::cout << "Attempting to withdraw 0..." << std::endl;
            acc5.withdraw(0);
            std::cout << "Test Case 5 FAILED (zero withdrawal)" << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cout << "Caught expected exception: " << e.what() << std::endl;
            std::cout << "Test Case 5 PASSED" << std::endl;
        }
    } catch(const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }

    return 0;
}