#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <optional>

class BankAccount {
private:
    long long balance;
    bool is_open;
    std::mutex mtx;

public:
    BankAccount(long long initial_balance) : balance(initial_balance), is_open(true) {}

    bool deposit(long long amount) {
        if (amount <= 0) return false;
        std::lock_guard<std::mutex> lock(mtx);
        if (!is_open) return false;
        if (balance > LLONG_MAX - amount) return false; // Overflow check
        balance += amount;
        return true;
    }

    bool withdraw(long long amount) {
        if (amount <= 0) return false;
        std::lock_guard<std::mutex> lock(mtx);
        if (!is_open || balance < amount) return false;
        balance -= amount;
        return true;
    }

    std::optional<long long> get_balance() {
        std::lock_guard<std::mutex> lock(mtx);
        if (!is_open) return std::nullopt;
        return balance;
    }

    void close() {
        std::lock_guard<std::mutex> lock(mtx);
        is_open = false;
    }
};

class Bank {
private:
    std::unordered_map<int, std::shared_ptr<BankAccount>> accounts;
    std::atomic<int> next_account_id{0};
    std::mutex map_mtx;

public:
    int open_account(long long initial_balance) {
        if (initial_balance < 0) return -1;
        int account_id = ++next_account_id;
        auto new_account = std::make_shared<BankAccount>(initial_balance);
        std::lock_guard<std::mutex> lock(map_mtx);
        accounts[account_id] = new_account;
        return account_id;
    }

    std::shared_ptr<BankAccount> get_account(int account_id) {
        std::lock_guard<std::mutex> lock(map_mtx);
        auto it = accounts.find(account_id);
        if (it != accounts.end()) {
            return it->second;
        }
        return nullptr;
    }
};

// Global bank object
Bank global_bank;

// Wrapper functions for C-style API if desired, or just use the bank object.
int openAccount(long long initialBalance) {
    return global_bank.open_account(initialBalance);
}

bool closeAccount(int accountId) {
    auto account = global_bank.get_account(accountId);
    if (account) {
        account->close();
        return true;
    }
    return false;
}

bool deposit(int accountId, long long amount) {
    auto account = global_bank.get_account(accountId);
    if (account) {
        return account->deposit(amount);
    }
    return false;
}

bool withdraw(int accountId, long long amount) {
    auto account = global_bank.get_account(accountId);
    if (account) {
        return account->withdraw(amount);
    }
    return false;
}

std::optional<long long> getBalance(int accountId) {
    auto account = global_bank.get_account(accountId);
    if (account) {
        return account->get_balance();
    }
    return std::nullopt;
}

void print_balance(const std::string& msg, int acc_id) {
    auto balance = getBalance(acc_id);
    std::cout << msg;
    if (balance) {
        std::cout << *balance << std::endl;
    } else {
        std::cout << "N/A (account closed or not found)" << std::endl;
    }
}

int main() {
    std::cout << "--- C++ Bank Test Cases ---" << std::endl;

    // Test Case 1: Simple deposit and withdrawal
    std::cout << "\n--- Test Case 1: Simple Operations ---" << std::endl;
    int acc1 = openAccount(100);
    print_balance("Opened account " + std::to_string(acc1) + " with balance: ", acc1);
    deposit(acc1, 50);
    print_balance("Deposited 50. New balance: ", acc1);
    withdraw(acc1, 30);
    print_balance("Withdrew 30. Final balance: ", acc1);

    // Test Case 2: Insufficient funds
    std::cout << "\n--- Test Case 2: Insufficient Funds ---" << std::endl;
    int acc2 = openAccount(50);
    print_balance("Opened account " + std::to_string(acc2) + " with balance: ", acc2);
    bool success = withdraw(acc2, 100);
    std::cout << "Attempted to withdraw 100. Success: " << (success ? "true" : "false") << std::endl;
    print_balance("Final balance: ", acc2);

    // Test Case 3: Operations on a closed account
    std::cout << "\n--- Test Case 3: Closed Account Operations ---" << std::endl;
    int acc3 = openAccount(200);
    print_balance("Opened account " + std::to_string(acc3) + " with balance: ", acc3);
    closeAccount(acc3);
    std::cout << "Account " << acc3 << " closed." << std::endl;
    std::cout << "Attempting deposit of 50. Success: " << (deposit(acc3, 50) ? "true" : "false") << std::endl;
    std::cout << "Attempting withdrawal of 20. Success: " << (withdraw(acc3, 20) ? "true" : "false") << std::endl;
    print_balance("Final balance: ", acc3);

    // Test Case 4: Concurrent deposits
    std::cout << "\n--- Test Case 4: Concurrent Deposits ---" << std::endl;
    int acc4 = openAccount(0);
    print_balance("Opened account " + std::to_string(acc4) + " with balance: ", acc4);
    std::vector<std::thread> deposit_threads;
    for (int i = 0; i < 10; ++i) {
        deposit_threads.emplace_back(deposit, acc4, 10);
    }
    for (auto& th : deposit_threads) {
        th.join();
    }
    print_balance("After 10 threads each deposited 10, final balance: ", acc4);
    
    // Test Case 5: Concurrent deposits and withdrawals
    std::cout << "\n--- Test Case 5: Concurrent Mix ---" << std::endl;
    int acc5 = openAccount(1000);
    print_balance("Opened account " + std::to_string(acc5) + " with balance: ", acc5);
    std::vector<std::thread> mix_threads;
    for (int i = 0; i < 5; ++i) {
        mix_threads.emplace_back(deposit, acc5, 20);
        mix_threads.emplace_back(withdraw, acc5, 30);
    }
    for (auto& th : mix_threads) {
        th.join();
    }
    // Expected: 1000 + (5*20) - (5*30) = 950
    print_balance("After 5 deposits of 20 and 5 withdrawals of 30, final balance: ", acc5);

    return 0;
}