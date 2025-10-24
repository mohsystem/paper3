
#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <iomanip>

class Task181 {
private:
    class Account {
    private:
        double balance;
        bool closed;
        mutable std::shared_mutex mutex;
        
    public:
        Account(double initialBalance) : balance(initialBalance), closed(false) {}
        
        bool deposit(double amount) {
            std::unique_lock<std::shared_mutex> lock(mutex);
            if (closed) {
                return false;
            }
            if (amount <= 0) {
                return false;
            }
            balance += amount;
            return true;
        }
        
        bool withdraw(double amount) {
            std::unique_lock<std::shared_mutex> lock(mutex);
            if (closed) {
                return false;
            }
            if (amount <= 0) {
                return false;
            }
            if (balance < amount) {
                return false;
            }
            balance -= amount;
            return true;
        }
        
        bool getBalance(double& outBalance) const {
            std::shared_lock<std::shared_mutex> lock(mutex);
            if (closed) {
                return false;
            }
            outBalance = balance;
            return true;
        }
        
        bool close() {
            std::unique_lock<std::shared_mutex> lock(mutex);
            if (closed) {
                return false;
            }
            closed = true;
            return true;
        }
        
        bool isClosed() const {
            std::shared_lock<std::shared_mutex> lock(mutex);
            return closed;
        }
    };
    
    std::unordered_map<std::string, std::shared_ptr<Account>> accounts;
    mutable std::mutex accountsMutex;
    
public:
    Task181() {}
    
    bool openAccount(const std::string& accountId, double initialBalance) {
        if (initialBalance < 0) {
            return false;
        }
        
        std::lock_guard<std::mutex> lock(accountsMutex);
        if (accounts.find(accountId) != accounts.end()) {
            return false;
        }
        accounts[accountId] = std::make_shared<Account>(initialBalance);
        return true;
    }
    
    bool closeAccount(const std::string& accountId) {
        std::shared_ptr<Account> account;
        {
            std::lock_guard<std::mutex> lock(accountsMutex);
            auto it = accounts.find(accountId);
            if (it == accounts.end()) {
                return false;
            }
            account = it->second;
        }
        return account->close();
    }
    
    bool deposit(const std::string& accountId, double amount) {
        std::shared_ptr<Account> account;
        {
            std::lock_guard<std::mutex> lock(accountsMutex);
            auto it = accounts.find(accountId);
            if (it == accounts.end()) {
                return false;
            }
            account = it->second;
        }
        return account->deposit(amount);
    }
    
    bool withdraw(const std::string& accountId, double amount) {
        std::shared_ptr<Account> account;
        {
            std::lock_guard<std::mutex> lock(accountsMutex);
            auto it = accounts.find(accountId);
            if (it == accounts.end()) {
                return false;
            }
            account = it->second;
        }
        return account->withdraw(amount);
    }
    
    bool getBalance(const std::string& accountId, double& outBalance) const {
        std::shared_ptr<Account> account;
        {
            std::lock_guard<std::mutex> lock(accountsMutex);
            auto it = accounts.find(accountId);
            if (it == accounts.end()) {
                return false;
            }
            account = it->second;
        }
        return account->getBalance(outBalance);
    }
};

int main() {
    Task181 bank;
    
    // Test Case 1: Open account and check balance
    std::cout << "Test 1: Open account" << std::endl;
    bool opened = bank.openAccount("ACC001", 1000.00);
    std::cout << "Account opened: " << std::boolalpha << opened << std::endl;
    double balance;
    if (bank.getBalance("ACC001", balance)) {
        std::cout << "Balance: " << std::fixed << std::setprecision(2) << balance << std::endl;
    }
    
    // Test Case 2: Deposit money
    std::cout << "\\nTest 2: Deposit money" << std::endl;
    bool deposited = bank.deposit("ACC001", 500.00);
    std::cout << "Deposit successful: " << deposited << std::endl;
    if (bank.getBalance("ACC001", balance)) {
        std::cout << "Balance: " << balance << std::endl;
    }
    
    // Test Case 3: Withdraw money
    std::cout << "\\nTest 3: Withdraw money" << std::endl;
    bool withdrawn = bank.withdraw("ACC001", 300.00);
    std::cout << "Withdrawal successful: " << withdrawn << std::endl;
    if (bank.getBalance("ACC001", balance)) {
        std::cout << "Balance: " << balance << std::endl;
    }
    
    // Test Case 4: Close account and try operations
    std::cout << "\\nTest 4: Close account" << std::endl;
    bool closed = bank.closeAccount("ACC001");
    std::cout << "Account closed: " << closed << std::endl;
    bool depositAfterClose = bank.deposit("ACC001", 100.00);
    std::cout << "Deposit after close: " << depositAfterClose << std::endl;
    bool hasBalance = bank.getBalance("ACC001", balance);
    std::cout << "Balance after close: " << (hasBalance ? "Available" : "Not available") << std::endl;
    
    // Test Case 5: Insufficient funds
    std::cout << "\\nTest 5: Insufficient funds" << std::endl;
    bank.openAccount("ACC002", 100.00);
    bool withdrawMore = bank.withdraw("ACC002", 200.00);
    std::cout << "Withdrawal (insufficient funds): " << withdrawMore << std::endl;
    if (bank.getBalance("ACC002", balance)) {
        std::cout << "Balance: " << balance << std::endl;
    }
    
    return 0;
}
