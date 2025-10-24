#include <bits/stdc++.h>
using namespace std;

class Task181 {
public:
    class Bank {
        struct Account {
            mutable std::mutex mtx;
            long long balance;
            bool closed;
            Account(long long b) : balance(b), closed(false) {}
        };
        std::unordered_map<long long, std::shared_ptr<Account>> accounts;
        std::mutex accountsMtx;
        std::atomic<long long> nextId{1};

    public:
        long long openAccount(long long initialCents) {
            if (initialCents < 0) return -1;
            long long id = nextId.fetch_add(1, std::memory_order_relaxed);
            auto acc = std::make_shared<Account>(initialCents);
            std::lock_guard<std::mutex> lg(accountsMtx);
            accounts[id] = acc;
            return id;
        }

        bool closeAccount(long long accountId) {
            std::shared_ptr<Account> acc;
            {
                std::lock_guard<std::mutex> lg(accountsMtx);
                auto it = accounts.find(accountId);
                if (it == accounts.end()) return false;
                acc = it->second;
            }
            std::lock_guard<std::mutex> lg(acc->mtx);
            if (acc->closed) return false;
            acc->closed = true;
            return true;
        }

        bool deposit(long long accountId, long long cents) {
            if (cents <= 0) return false;
            std::shared_ptr<Account> acc;
            {
                std::lock_guard<std::mutex> lg(accountsMtx);
                auto it = accounts.find(accountId);
                if (it == accounts.end()) return false;
                acc = it->second;
            }
            std::lock_guard<std::mutex> lg(acc->mtx);
            if (acc->closed) return false;
            if (acc->balance > LLONG_MAX - cents) return false; // overflow
            acc->balance += cents;
            return true;
        }

        bool withdraw(long long accountId, long long cents) {
            if (cents <= 0) return false;
            std::shared_ptr<Account> acc;
            {
                std::lock_guard<std::mutex> lg(accountsMtx);
                auto it = accounts.find(accountId);
                if (it == accounts.end()) return false;
                acc = it->second;
            }
            std::lock_guard<std::mutex> lg(acc->mtx);
            if (acc->closed) return false;
            if (cents > acc->balance) return false;
            acc->balance -= cents;
            return true;
        }

        // Returns pair<bool success, long long balanceOut>
        pair<bool, long long> getBalance(long long accountId) {
            std::shared_ptr<Account> acc;
            {
                std::lock_guard<std::mutex> lg(accountsMtx);
                auto it = accounts.find(accountId);
                if (it == accounts.end()) return {false, 0};
                acc = it->second;
            }
            std::lock_guard<std::mutex> lg(acc->mtx);
            if (acc->closed) return {false, 0};
            return {true, acc->balance};
        }
    };
};

static void runConcurrent(function<void()> fn, int threads) {
    vector<thread> ts;
    atomic<int> ready{0};
    atomic<bool> start{false};
    ts.reserve(threads);
    for (int i = 0; i < threads; ++i) {
        ts.emplace_back([&](){
            ready.fetch_add(1, memory_order_relaxed);
            while (!start.load(memory_order_acquire)) { this_thread::yield(); }
            fn();
        });
    }
    while (ready.load(memory_order_acquire) != threads) this_thread::yield();
    start.store(true, memory_order_release);
    for (auto &t : ts) t.join();
}

int main() {
    Task181::Bank bank;

    // Test 1
    auto a1 = bank.openAccount(10000);
    bool d1 = bank.deposit(a1, 5000);
    bool w1 = bank.withdraw(a1, 2000);
    auto b1 = bank.getBalance(a1);
    cout << "Test1: " << (d1 && w1 && b1.first && b1.second == 13000) << "\n";

    // Test 2: Concurrent deposits
    auto a2 = bank.openAccount(0);
    int threads2 = 10, perOps2 = 1000;
    long long perOp2 = 1000;
    runConcurrent([&](){
        for (int i = 0; i < perOps2; ++i) bank.deposit(a2, perOp2);
    }, threads2);
    auto b2 = bank.getBalance(a2);
    long long expected2 = 1LL * threads2 * perOps2 * perOp2;
    cout << "Test2: " << (b2.first && b2.second == expected2) << "\n";

    // Test 3: Mixed operations
    auto a3 = bank.openAccount(1'000'000);
    int threads3 = 8, perOps3 = 5000;
    runConcurrent([&](){
        for (int i = 0; i < perOps3; ++i) {
            bank.deposit(a3, 100);
            bank.withdraw(a3, 100);
        }
    }, threads3);
    auto b3 = bank.getBalance(a3);
    cout << "Test3: " << (b3.first && b3.second == 1'000'000) << "\n";

    // Test 4: Close and fail
    auto a4 = bank.openAccount(50'000);
    bool c4 = bank.closeAccount(a4);
    bool d4 = bank.deposit(a4, 1000);
    bool w4 = bank.withdraw(a4, 1000);
    auto b4 = bank.getBalance(a4);
    cout << "Test4: " << (c4 && !d4 && !w4 && !b4.first) << "\n";

    // Test 5: Insufficient and invalid
    auto a5 = bank.openAccount(5000);
    bool w5a = bank.withdraw(a5, 10000);
    bool d5a = bank.deposit(a5, -100);
    bool w5b = bank.withdraw(a5, 0);
    auto b5 = bank.getBalance(a5);
    cout << "Test5: " << (!w5a && !d5a && !w5b && b5.first && b5.second == 5000) << "\n";

    return 0;
}