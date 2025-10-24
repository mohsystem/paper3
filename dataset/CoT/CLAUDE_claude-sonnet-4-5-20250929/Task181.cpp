
#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <thread>
#include <vector>
#include <climits>

class Task181 {
private:
    class BankAccount {
    private:
        std::string accountId;
        long long balance;
        bool isClosed;
        mutable std::recursive_mutex mtx;

    public:
        BankAccount(const std::string& id, long long initialBalance) 
            : accountId(id), balance(initialBalance), isClosed(false) {
            if (initialBalance < 0) {
                throw std::invalid_argument("Initial balance cannot be negative");
            }
        }

        bool deposit(long long amount) {
            if (amount <= 0) {
                return false;
            }

            std::lock_guard<std::recursive_mutex> lock(mtx);
            if (isClosed) {
                return false;
            }
            // Check for overflow
            if (balance > LLONG_MAX - amount) {
                return false;
            }
            balance += amount;
            return true;
        }

        bool withdraw(long long amount) {
            if (amount <= 0) {
                return false;
            }

            std::lock_guard<std::recursive_mutex> lock(mtx);
            if (isClosed) {
                return false;
            }
            if (balance < amount) {
                return false;
            }
            balance -= amount;
            return true;
        }

        long long getBalance() const {
            std::lock_guard<std::recursive_mutex> lock(mtx);
            return isClosed ? -1 : balance;
        }

        bool close() {
            std::lock_guard<std::recursive_mutex> lock(mtx);
            if (isClosed) {
                return false;
            }
            isClosed = true;
            return true;
        }

        bool isOpen() const {
            std::lock_guard<std::recursive_mutex> lock(mtx);
            return !isClosed;
        }
    };

    class BankingSystem {
    private:
        std::unordered_map<std::string, std::shared_ptr<BankAccount>> accounts;
        mutable std::mutex accountsMtx;

    public:
        bool openAccount(const std::string& accountId, long long initialBalance) {
            if (accountId.empty()) {
                return false;
            }

            std::lock_guard<std::mutex> lock(accountsMtx);
            if (accounts.find(accountId) != accounts.end()) {
                return false;
            }
            try {
                accounts[accountId] = std::make_shared<BankAccount>(accountId, initialBalance);
                return true;
            } catch (const std::invalid_argument&) {
                return false;
            }
        }

        bool closeAccount(const std::string& accountId) {
            std::lock_guard<std::mutex> lock(accountsMtx);
            auto it = accounts.find(accountId);
            return it != accounts.end() && it->second->close();
        }

        bool deposit(const std::string& accountId, long long amount) {
            std::lock_guard<std::mutex> lock(accountsMtx);
            auto it = accounts.find(accountId);
            return it != accounts.end() && it->second->deposit(amount);
        }

        bool withdraw(const std::string& accountId, long long amount) {
            std::lock_guard<std::mutex> lock(accountsMtx);
            auto it = accounts.find(accountId);
            return it != accounts.end() && it->second->withdraw(amount);
        }

        long long getBalance(const std::string& accountId) {
            std::lock_guard<std::mutex> lock(accountsMtx);
            auto it = accounts.find(accountId);
            return it != accounts.end() ? it->second->getBalance() : -1;
        }
    };

public:
    static void runTests() {
        BankingSystem bank;

        // Test Case 1: Open account and deposit
        std::cout << "Test 1 - Open account and deposit:" << std::endl;
        bank.openAccount("ACC001", 1000);
        bank.deposit("ACC001", 500);
        std::cout << "Balance: " << bank.getBalance("ACC001") << std::endl; // Expected: 1500

        // Test Case 2: Withdraw from account
        std::cout << "\\nTest 2 - Withdraw:" << std::endl;
        bank.withdraw("ACC001", 300);
        std::cout << "Balance: " << bank.getBalance("ACC001") << std::endl; // Expected: 1200

        // Test Case 3: Close account and try operations
        std::cout << "\\nTest 3 - Close account:" << std::endl;
        bank.closeAccount("ACC001");
        bool depositResult = bank.deposit("ACC001", 100);
        std::cout << "Deposit after close: " << depositResult << std::endl; // Expected: 0
        std::cout << "Balance after close: " << bank.getBalance("ACC001") << std::endl; // Expected: -1

        // Test Case 4: Insufficient funds
        std::cout << "\\nTest 4 - Insufficient funds:" << std::endl;
        bank.openAccount("ACC002", 100);
        bool withdrawResult = bank.withdraw("ACC002", 200);
        std::cout << "Withdraw result: " << withdrawResult << std::endl; // Expected: 0

        // Test Case 5: Concurrent operations
        std::cout << "\\nTest 5 - Concurrent operations:" << std::endl;
        bank.openAccount("ACC003", 1000);
        
        std::thread t1([&bank]() {
            for (int i = 0; i < 100; i++) {
                bank.deposit("ACC003", 10);
            }
        });
        
        std::thread t2([&bank]() {
            for (int i = 0; i < 100; i++) {
                bank.withdraw("ACC003", 5);
            }
        });
        
        t1.join();
        t2.join();
        std::cout << "Final balance: " << bank.getBalance("ACC003") << std::endl; // Expected: 1500
    }
};

int main() {
    Task181::runTests();
    return 0;
}
