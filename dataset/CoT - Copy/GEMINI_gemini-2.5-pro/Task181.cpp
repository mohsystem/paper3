#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <memory>

class BankAccount {
private:
    long balance;
    bool isOpen;
    std::mutex mtx;

public:
    BankAccount(long initialBalance) {
        if (initialBalance < 0) {
            throw std::invalid_argument("Initial balance cannot be negative.");
        }
        balance = initialBalance;
        isOpen = true;
    }

    long getBalance() {
        std::lock_guard<std::mutex> lock(mtx);
        if (!isOpen) {
            throw std::runtime_error("Account is closed.");
        }
        return balance;
    }

    void deposit(long amount) {
        if (amount <= 0) {
            throw std::invalid_argument("Deposit amount must be positive.");
        }
        std::lock_guard<std::mutex> lock(mtx);
        if (!isOpen) {
            throw std::runtime_error("Account is closed.");
        }
        balance += amount;
    }

    void withdraw(long amount) {
        if (amount <= 0) {
            throw std::invalid_argument("Withdrawal amount must be positive.");
        }
        std::lock_guard<std::mutex> lock(mtx);
        if (!isOpen) {
            throw std::runtime_error("Account is closed.");
        }
        if (balance < amount) {
            throw std::runtime_error("Insufficient funds.");
        }
        balance -= amount;
    }

    void close() {
        std::lock_guard<std::mutex> lock(mtx);
        isOpen = false;
    }
};

class Task181 {
public:
    static void run_tests() {
        // Test Case 1: Basic Operations
        std::cout << "--- Test Case 1: Basic Operations ---" << std::endl;
        try {
            BankAccount account1(1000);
            std::cout << "Initial Balance: " << account1.getBalance() << std::endl;
            account1.deposit(500);
            std::cout << "Balance after deposit: " << account1.getBalance() << std::endl;
            account1.withdraw(200);
            std::cout << "Balance after withdrawal: " << account1.getBalance() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        // Test Case 2: Insufficient Funds
        std::cout << "\n--- Test Case 2: Insufficient Funds ---" << std::endl;
        try {
            BankAccount account2(100);
            std::cout << "Current Balance: " << account2.getBalance() << std::endl;
            std::cout << "Attempting to withdraw 200..." << std::endl;
            account2.withdraw(200);
        } catch (const std::exception& e) {
            std::cout << "Caught expected error: " << e.what() << std::endl;
        }

        // Test Case 3: Operations on a closed account
        std::cout << "\n--- Test Case 3: Operations on Closed Account ---" << std::endl;
        BankAccount account3(500);
        account3.close();
        std::cout << "Account closed." << std::endl;
        try {
            account3.deposit(100);
        } catch (const std::exception& e) {
            std::cout << "Caught expected error on deposit: " << e.what() << std::endl;
        }
        try {
            account3.withdraw(50);
        } catch (const std::exception& e) {
            std::cout << "Caught expected error on withdraw: " << e.what() << std::endl;
        }
        try {
            account3.getBalance();
        } catch (const std::exception& e) {
            std::cout << "Caught expected error on get_balance: " << e.what() << std::endl;
        }

        // Test Case 4: Concurrent Transactions
        std::cout << "\n--- Test Case 4: Concurrent Transactions ---" << std::endl;
        auto account4 = std::make_shared<BankAccount>(10000);
        std::vector<std::thread> threads;
        
        auto worker = [&](long deposit_amount, long withdraw_amount, int iterations) {
            for (int i = 0; i < iterations; ++i) {
                try {
                    account4->deposit(deposit_amount);
                    account4->withdraw(withdraw_amount);
                } catch (const std::exception& e) {
                    // This might happen if funds get low, but for this test we expect it to succeed
                }
            }
        };

        for (int i = 0; i < 10; ++i) {
            threads.emplace_back(worker, 10, 5, 100); // 10 threads, each doing 100 iterations
        }
        for (auto& t : threads) {
            t.join();
        }
        
        std::cout << "Initial concurrent balance: 10000" << std::endl;
        std::cout << "After 10 threads each doing 100 deposits of 10 and 100 withdrawals of 5..." << std::endl;
        // Expected: 10000 + 10 * 100 * (10 - 5) = 10000 + 5000 = 15000
        try {
            std::cout << "Final concurrent balance: " << account4->getBalance() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error getting final balance: " << e.what() << std::endl;
        }

        // Test Case 5: Opening with negative balance
        std::cout << "\n--- Test Case 5: Opening with Negative Balance ---" << std::endl;
        try {
            BankAccount account5(-100);
        } catch (const std::exception& e) {
            std::cout << "Caught expected error: " << e.what() << std::endl;
        }
    }
};

int main() {
    Task181::run_tests();
    return 0;
}