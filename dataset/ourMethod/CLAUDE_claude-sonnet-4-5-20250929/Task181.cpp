
#include <iostream>
#include <map>
#include <mutex>
#include <memory>
#include <string>
#include <atomic>
#include <stdexcept>
#include <iomanip>

// Thread-safe bank account class with RAII and smart pointer management
class BankAccount {
private:
    std::string accountId;
    double balance;
    bool closed;
    mutable std::mutex accountMutex;  // Protects balance and closed state
    
public:
    // Constructor initializes account with ID and initial balance
    explicit BankAccount(const std::string& id, double initialBalance = 0.0)
        : accountId(id), balance(initialBalance), closed(false) {
        // Validate initial balance is non-negative
        if (initialBalance < 0.0) {
            throw std::invalid_argument("Initial balance cannot be negative");
        }
    }
    
    // Prevent copying to avoid threading issues
    BankAccount(const BankAccount&) = delete;
    BankAccount& operator=(const BankAccount&) = delete;
    
    // Deposit money with thread-safe operations
    bool deposit(double amount) {
        if (amount <= 0.0) {
            return false;  // Reject non-positive deposits
        }
        
        std::lock_guard<std::mutex> lock(accountMutex);
        
        // Check if account is closed - fail gracefully
        if (closed) {
            return false;
        }
        
        // Prevent overflow
        if (balance + amount < balance) {
            return false;
        }
        
        balance += amount;
        return true;
    }
    
    // Withdraw money with thread-safe operations
    bool withdraw(double amount) {
        if (amount <= 0.0) {
            return false;  // Reject non-positive withdrawals
        }
        
        std::lock_guard<std::mutex> lock(accountMutex);
        
        // Check if account is closed - fail gracefully
        if (closed) {
            return false;
        }
        
        // Check sufficient funds atomically with withdrawal
        if (balance < amount) {
            return false;
        }
        
        balance -= amount;
        return true;
    }
    
    // Get balance with thread-safe read
    double getBalance() const {
        std::lock_guard<std::mutex> lock(accountMutex);
        return balance;
    }
    
    // Close account with thread-safe state change
    bool close() {
        std::lock_guard<std::mutex> lock(accountMutex);
        
        if (closed) {
            return false;  // Already closed
        }
        
        closed = true;
        return true;
    }
    
    // Check if account is closed
    bool isClosed() const {
        std::lock_guard<std::mutex> lock(accountMutex);
        return closed;
    }
    
    std::string getId() const {
        return accountId;
    }
};

// Thread-safe bank system managing multiple accounts
class BankSystem {
private:
    std::map<std::string, std::shared_ptr<BankAccount>> accounts;
    mutable std::mutex systemMutex;  // Protects the accounts map
    std::atomic<int> nextAccountId{1000};
    
public:
    // Open new account with unique ID generation
    std::string openAccount(double initialBalance = 0.0) {
        // Validate initial balance
        if (initialBalance < 0.0) {
            return "";  // Return empty string on failure
        }
        
        // Generate unique account ID atomically
        int accountNum = nextAccountId.fetch_add(1);
        std::string accountId = "ACC" + std::to_string(accountNum);
        
        try {
            auto account = std::make_shared<BankAccount>(accountId, initialBalance);
            
            std::lock_guard<std::mutex> lock(systemMutex);
            accounts[accountId] = account;
            
            return accountId;
        } catch (const std::exception&) {
            return "";  // Return empty string on failure
        }
    }
    
    // Close account by ID
    bool closeAccount(const std::string& accountId) {
        std::shared_ptr<BankAccount> account;
        
        {
            std::lock_guard<std::mutex> lock(systemMutex);
            auto it = accounts.find(accountId);
            if (it == accounts.end()) {
                return false;  // Account not found
            }
            account = it->second;
        }
        
        return account->close();
    }
    
    // Deposit to account
    bool deposit(const std::string& accountId, double amount) {
        std::shared_ptr<BankAccount> account;
        
        {
            std::lock_guard<std::mutex> lock(systemMutex);
            auto it = accounts.find(accountId);
            if (it == accounts.end()) {
                return false;  // Account not found
            }
            account = it->second;
        }
        
        return account->deposit(amount);
    }
    
    // Withdraw from account
    bool withdraw(const std::string& accountId, double amount) {
        std::shared_ptr<BankAccount> account;
        
        {
            std::lock_guard<std::mutex> lock(systemMutex);
            auto it = accounts.find(accountId);
            if (it == accounts.end()) {
                return false;  // Account not found
            }
            account = it->second;
        }
        
        return account->withdraw(amount);
    }
    
    // Get account balance
    double getBalance(const std::string& accountId) const {
        std::shared_ptr<BankAccount> account;
        
        {
            std::lock_guard<std::mutex> lock(systemMutex);
            auto it = accounts.find(accountId);
            if (it == accounts.end()) {
                return -1.0;  // Return -1 to indicate account not found
            }
            account = it->second;
        }
        
        return account->getBalance();
    }
};

int main() {
    BankSystem bank;
    
    std::cout << "=== Test Case 1: Open Account and Deposit ===" << std::endl;
    std::string acc1 = bank.openAccount(1000.0);
    std::cout << "Opened account: " << acc1 << std::endl;
    std::cout << "Initial balance: $" << std::fixed << std::setprecision(2) 
              << bank.getBalance(acc1) << std::endl;
    bool depositResult = bank.deposit(acc1, 500.0);
    std::cout << "Deposit $500: " << (depositResult ? "Success" : "Failed") << std::endl;
    std::cout << "New balance: $" << bank.getBalance(acc1) << std::endl;
    std::cout << std::endl;
    
    std::cout << "=== Test Case 2: Withdraw from Account ===" << std::endl;
    bool withdrawResult = bank.withdraw(acc1, 300.0);
    std::cout << "Withdraw $300: " << (withdrawResult ? "Success" : "Failed") << std::endl;
    std::cout << "Balance after withdrawal: $" << bank.getBalance(acc1) << std::endl;
    std::cout << std::endl;
    
    std::cout << "=== Test Case 3: Insufficient Funds ===" << std::endl;
    withdrawResult = bank.withdraw(acc1, 2000.0);
    std::cout << "Withdraw $2000 (insufficient): " 
              << (withdrawResult ? "Success" : "Failed") << std::endl;
    std::cout << "Balance unchanged: $" << bank.getBalance(acc1) << std::endl;
    std::cout << std::endl;
    
    std::cout << "=== Test Case 4: Close Account ===" << std::endl;
    bool closeResult = bank.closeAccount(acc1);
    std::cout << "Close account: " << (closeResult ? "Success" : "Failed") << std::endl;
    depositResult = bank.deposit(acc1, 100.0);
    std::cout << "Deposit to closed account: " 
              << (depositResult ? "Success" : "Failed") << std::endl;
    std::cout << std::endl;
    
    std::cout << "=== Test Case 5: Multiple Accounts ===" << std::endl;
    std::string acc2 = bank.openAccount(2000.0);
    std::string acc3 = bank.openAccount(3000.0);
    std::cout << "Opened account: " << acc2 << " with balance: $" 
              << bank.getBalance(acc2) << std::endl;
    std::cout << "Opened account: " << acc3 << " with balance: $" 
              << bank.getBalance(acc3) << std::endl;
    bank.withdraw(acc2, 500.0);
    bank.deposit(acc3, 1000.0);
    std::cout << "After operations:" << std::endl;
    std::cout << acc2 << " balance: $" << bank.getBalance(acc2) << std::endl;
    std::cout << acc3 << " balance: $" << bank.getBalance(acc3) << std::endl;
    
    return 0;
}
