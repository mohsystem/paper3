#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <atomic>

static const long long ERR_NO_ACCOUNT_OR_CLOSED = -1LL;
static const long long ERR_INSUFFICIENT_FUNDS = -2LL;
static const long long ERR_INVALID_INPUT = -3LL;

struct Account {
    std::mutex mtx;
    long long balance;
    bool closed;
    explicit Account(long long initial) : balance(initial), closed(false) {}
};

class Bank {
public:
    bool openAccount(const std::string& id, long long initialCents) {
        if (!validId(id) || initialCents < 0) return false;
        std::lock_guard<std::mutex> g(mapMtx);
        auto it = accounts.find(id);
        if (it != accounts.end()) return false;
        accounts.emplace(id, std::make_shared<Account>(initialCents));
        return true;
    }

    long long getBalance(const std::string& id) {
        if (!validId(id)) return ERR_INVALID_INPUT;
        auto acc = getAccount(id);
        if (!acc) return ERR_NO_ACCOUNT_OR_CLOSED;
        std::lock_guard<std::mutex> g(acc->mtx);
        if (acc->closed) return ERR_NO_ACCOUNT_OR_CLOSED;
        return acc->balance;
    }

    long long deposit(const std::string& id, long long amountCents) {
        if (!validId(id) || amountCents < 0) return ERR_INVALID_INPUT;
        auto acc = getAccount(id);
        if (!acc) return ERR_NO_ACCOUNT_OR_CLOSED;
        std::lock_guard<std::mutex> g(acc->mtx);
        if (acc->closed) return ERR_NO_ACCOUNT_OR_CLOSED;
        long long newBal = acc->balance + amountCents;
        if (newBal < 0) return ERR_INVALID_INPUT; // overflow guard
        acc->balance = newBal;
        return acc->balance;
    }

    long long withdraw(const std::string& id, long long amountCents) {
        if (!validId(id) || amountCents < 0) return ERR_INVALID_INPUT;
        auto acc = getAccount(id);
        if (!acc) return ERR_NO_ACCOUNT_OR_CLOSED;
        std::lock_guard<std::mutex> g(acc->mtx);
        if (acc->closed) return ERR_NO_ACCOUNT_OR_CLOSED;
        if (acc->balance < amountCents) return ERR_INSUFFICIENT_FUNDS;
        acc->balance -= amountCents;
        return acc->balance;
    }

    bool closeAccount(const std::string& id) {
        if (!validId(id)) return false;
        auto acc = getAccount(id);
        if (!acc) return false;
        std::lock_guard<std::mutex> g(acc->mtx);
        if (acc->closed || acc->balance != 0) return false;
        acc->closed = true;
        return true;
    }

private:
    std::mutex mapMtx;
    std::unordered_map<std::string, std::shared_ptr<Account>> accounts;

    static bool validId(const std::string& id) {
        if (id.empty() || id.size() > 32) return false;
        for (char c : id) {
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')) {
                return false;
            }
        }
        return true;
    }

    std::shared_ptr<Account> getAccount(const std::string& id) {
        std::lock_guard<std::mutex> g(mapMtx);
        auto it = accounts.find(id);
        if (it == accounts.end()) return nullptr;
        return it->second;
    }
};

int main() {
    Bank bank;

    // Test 1: Open and balance check
    bool opened1 = bank.openAccount("alice", 10000);
    long long bal1 = bank.getBalance("alice");
    std::cout << "Test1: opened=" << opened1 << " balance=" << bal1 << "\n";

    // Test 2: Deposit
    long long bal2 = bank.deposit("alice", 2500);
    std::cout << "Test2: deposit newBalance=" << bal2 << "\n";

    // Test 3: Withdraw success
    long long bal3 = bank.withdraw("alice", 5000);
    std::cout << "Test3: withdraw newBalance=" << bal3 << "\n";

    // Test 4: Withdraw insufficient
    long long w4 = bank.withdraw("alice", 10000);
    long long bal4 = bank.getBalance("alice");
    std::cout << "Test4: withdrawResult=" << w4 << " balanceAfter=" << bal4 << "\n";

    // Test 5: Concurrency + close behavior
    std::string accId = "bob";
    bool opened2 = bank.openAccount(accId, 0);
    int depositThreads = 4;
    int depositIters = 1000;
    long long depositAmt = 100;
    int withdrawThreads = 2;
    int withdrawIters = 500;
    long long withdrawAmt = 50;
    std::vector<std::thread> threads;
    std::atomic<bool> startFlag{false};

    for (int i = 0; i < depositThreads; ++i) {
        threads.emplace_back([&]() {
            while (!startFlag.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            for (int j = 0; j < depositIters; ++j) {
                bank.deposit(accId, depositAmt);
            }
        });
    }
    for (int i = 0; i < withdrawThreads; ++i) {
        threads.emplace_back([&]() {
            while (!startFlag.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            for (int j = 0; j < withdrawIters; ++j) {
                long long r = bank.withdraw(accId, withdrawAmt);
                (void)r;
            }
        });
    }
    startFlag.store(true, std::memory_order_release);
    for (auto& t : threads) t.join();

    long long finalExpected = (long long)depositThreads * depositIters * depositAmt - (long long)withdrawThreads * withdrawIters * withdrawAmt;
    long long bal5 = bank.getBalance(accId);
    bool closeAttemptNonZero = bank.closeAccount(accId);
    long long withdrawAll = bal5 >= 0 ? bank.withdraw(accId, bal5) : bal5;
    bool closeAttemptZero = bank.closeAccount(accId);
    long long postCloseDeposit = bank.deposit(accId, 100);
    std::cout << "Test5: opened=" << opened2
              << " expectedBalance=" << finalExpected
              << " actualBalance=" << bal5
              << " closeNonZero=" << closeAttemptNonZero
              << " withdrawAllRes=" << withdrawAll
              << " closeZero=" << closeAttemptZero
              << " depositAfterCloseRes=" << postCloseDeposit
              << "\n";
    return 0;
}