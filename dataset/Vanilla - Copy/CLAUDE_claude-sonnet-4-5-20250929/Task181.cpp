
#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <vector>

class Task181 {
private:
    class BankAccount {
    private:
        double balance;
        bool closed;
        mutable std::mutex mtx;

    public:
        BankAccount(double initialBalance) 
            : balance(initialBalance), closed(false) {}

        bool deposit(double amount) {
            std::lock_guard<std::mutex> lock(mtx);
            if (closed || amount <= 0) {
                return false;
            }
            balance += amount;
            return true;
        }

        bool withdraw(double amount) {
            std::lock_guard<std::mutex> lock(mtx);
            if (closed || amount <= 0 || amount > balance) {
                return false;
            }
            balance -= amount;
            return true;
        }

        double getBalance() const {
            std::lock_guard<std::mutex> lock(mtx);
            if (closed) {
                return -1;
            }
            return balance;
        }

        bool close() {
            std::lock_guard<std::mutex> lock(mtx);
            if (closed) {
                return false;
            }
            closed = true;
            return true;
        }

        bool isClosed() const {
            std::lock_guard<std::mutex> lock(mtx);
            return closed;
        }
    };

    std::unordered_map<std::string, BankAccount*> accounts;
    mutable std::mutex accountsMtx;

public:
    Task181() {}

    ~Task181() {
        for (auto& pair : accounts) {
            delete pair.second;
        }
    }

    bool openAccount(const std::string& accountId, double initialBalance) {
        if (accountId.empty() || initialBalance < 0) {
            return false;
        }
        
        std::lock_guard<std::mutex> lock(accountsMtx);
        if (accounts.find(accountId) != accounts.end()) {
            return false;
        }
        accounts[accountId] = new BankAccount(initialBalance);
        return true;
    }

    bool closeAccount(const std::string& accountId) {
        std::lock_guard<std::mutex> lock(accountsMtx);
        auto it = accounts.find(accountId);
        if (it == accounts.end()) {
            return false;
        }
        return it->second->close();
    }

    bool deposit(const std::string& accountId, double amount) {
        std::lock_guard<std::mutex> lock(accountsMtx);
        auto it = accounts.find(accountId);
        if (it == accounts.end()) {
            return false;
        }
        return it->second->deposit(amount);
    }

    bool withdraw(const std::string& accountId, double amount) {
        std::lock_guard<std::mutex> lock(accountsMtx);
        auto it = accounts.find(accountId);
        if (it == accounts.end()) {
            return false;
        }
        return it->second->withdraw(amount);
    }

    double getBalance(const std::string& accountId) const {
        std::lock_guard<std::mutex> lock(accountsMtx);
        auto it = accounts.find(accountId);
        if (it == accounts.end()) {
            return -1;
        }
        return it->second->getBalance();
    }
};

int main() {
    Task181 bank;

    // Test Case 1: Open account and deposit
    std::cout << "Test Case 1: Open account and deposit" << std::endl;
    bank.openAccount("ACC001", 1000.0);
    bank.deposit("ACC001", 500.0);
    std::cout << "Balance: " << bank.getBalance("ACC001") << std::endl;

    // Test Case 2: Withdraw from account
    std::cout << "\\nTest Case 2: Withdraw from account" << std::endl;
    bank.withdraw("ACC001", 300.0);
    std::cout << "Balance: " << bank.getBalance("ACC001") << std::endl;

    // Test Case 3: Close account and try operations
    std::cout << "\\nTest Case 3: Close account and try operations" << std::endl;
    bank.closeAccount("ACC001");
    std::cout << "Deposit after close: " << bank.deposit("ACC001", 100.0) << std::endl;
    std::cout << "Balance after close: " << bank.getBalance("ACC001") << std::endl;

    // Test Case 4: Concurrent operations
    std::cout << "\\nTest Case 4: Concurrent operations" << std::endl;
    bank.openAccount("ACC002", 5000.0);
    
    std::thread t1([&bank]() {
        for (int i = 0; i < 100; i++) {
            bank.deposit("ACC002", 10.0);
        }
    });
    
    std::thread t2([&bank]() {
        for (int i = 0; i < 100; i++) {
            bank.withdraw("ACC002", 5.0);
        }
    });
    
    t1.join();
    t2.join();
    std::cout << "Balance after concurrent ops: " << bank.getBalance("ACC002") << std::endl;

    // Test Case 5: Invalid operations
    std::cout << "\\nTest Case 5: Invalid operations" << std::endl;
    std::cout << "Open duplicate account: " << bank.openAccount("ACC002", 1000.0) << std::endl;
    std::cout << "Withdraw from non-existent: " << bank.withdraw("ACC999", 100.0) << std::endl;
    std::cout << "Overdraw: " << bank.withdraw("ACC002", 10000.0) << std::endl;

    return 0;
}
