#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <stdexcept>
#include <atomic>

class Account {
private:
    double balance;
    bool is_open;
    std::mutex mtx;

public:
    explicit Account(double initial_balance) {
        if (initial_balance < 0) {
            throw std::invalid_argument("Initial balance cannot be negative.");
        }
        balance = initial_balance;
        is_open = true;
    }

    // Deleted copy constructor and assignment operator to prevent accidental copying
    Account(const Account&) = delete;
    Account& operator=(const Account&) = delete;

    void deposit(double amount) {
        if (amount <= 0) {
            throw std::invalid_argument("Deposit amount must be positive.");
        }
        std::lock_guard<std::mutex> lock(mtx);
        if (!is_open) {
            throw std::runtime_error("Account is closed.");
        }
        balance += amount;
    }

    void withdraw(double amount) {
        if (amount <= 0) {
            throw std::invalid_argument("Withdrawal amount must be positive.");
        }
        std::lock_guard<std::mutex> lock(mtx);
        if (!is_open) {
            throw std::runtime_error("Account is closed.");
        }
        if (balance < amount) {
            throw std::runtime_error("Insufficient funds.");
        }
        balance -= amount;
    }

    double get_balance() {
        std::lock_guard<std::mutex> lock(mtx);
        if (!is_open) {
            throw std::runtime_error("Account is closed.");
        }
        return balance;
    }

    void close() {
        std::lock_guard<std::mutex> lock(mtx);
        is_open = false;
    }
};

class Bank {
private:
    std::unordered_map<long, std::shared_ptr<Account>> accounts;
    std::atomic<long> next_account_id{1};
    std::mutex accounts_map_mutex;

    std::shared_ptr<Account> get_account(long account_id) {
        std::lock_guard<std::mutex> lock(accounts_map_mutex);
        auto it = accounts.find(account_id);
        if (it == accounts.end()) {
            throw std::invalid_argument("Account not found.");
        }
        return it->second;
    }

public:
    long open_account(double initial_balance) {
        long account_id = next_account_id.fetch_add(1);
        auto account = std::make_shared<Account>(initial_balance);
        std::lock_guard<std::mutex> lock(accounts_map_mutex);
        accounts[account_id] = account;
        return account_id;
    }

    void deposit(long account_id, double amount) {
        get_account(account_id)->deposit(amount);
    }

    void withdraw(long account_id, double amount) {
        get_account(account_id)->withdraw(amount);
    }

    double get_balance(long account_id) {
        return get_account(account_id)->get_balance();
    }

    void close_account(long account_id) {
        get_account(account_id)->close();
    }
};


int main() {
    Bank bank;
    std::cout << "--- Bank System Test ---" << std::endl;

    // Test Case 1: Open an account and check initial balance
    std::cout << "\n--- Test Case 1: Open Account ---" << std::endl;
    long acc1 = bank.open_account(100.0);
    std::cout << "Opened account " << acc1 << " with balance: " << bank.get_balance(acc1) << std::endl;

    // Test Case 2: Deposit money
    std::cout << "\n--- Test Case 2: Deposit ---" << std::endl;
    std::cout << "Depositing 50.0 into account " << acc1 << std::endl;
    bank.deposit(acc1, 50.0);
    std::cout << "New balance for account " << acc1 << ": " << bank.get_balance(acc1) << std::endl;

    // Test Case 3: Withdraw money
    std::cout << "\n--- Test Case 3: Withdraw ---" << std::endl;
    std::cout << "Withdrawing 30.0 from account " << acc1 << std::endl;
    bank.withdraw(acc1, 30.0);
    std::cout << "New balance for account " << acc1 << ": " << bank.get_balance(acc1) << std::endl;

    // Test Case 4: Attempt to withdraw more than available funds
    std::cout << "\n--- Test Case 4: Insufficient Funds ---" << std::endl;
    try {
        std::cout << "Attempting to withdraw 200.0 from account " << acc1 << std::endl;
        bank.withdraw(acc1, 200.0);
    } catch (const std::runtime_error& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
        std::cout << "Balance remains: " << bank.get_balance(acc1) << std::endl;
    }

    // Test Case 5: Close account and attempt a transaction
    std::cout << "\n--- Test Case 5: Closed Account ---" << std::endl;
    std::cout << "Closing account " << acc1 << std::endl;
    bank.close_account(acc1);
    try {
        std::cout << "Attempting to deposit 10.0 into closed account " << acc1 << std::endl;
        bank.deposit(acc1, 10.0);
    } catch (const std::runtime_error& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }
    try {
        std::cout << "Attempting to get balance of closed account " << acc1 << std::endl;
        bank.get_balance(acc1);
    } catch (const std::runtime_error& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }

    return 0;
}