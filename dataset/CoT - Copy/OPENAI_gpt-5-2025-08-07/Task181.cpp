// Chain-of-Through process in code comments:
// 1) Problem understanding: Thread-safe bank account management with concurrent access.
// 2) Security requirements: Input validation, overflow checks, closed/non-existing account handling.
// 3) Secure coding generation: Mutexes per account, map mutex for lookup; no deadlocks by locking one mutex at a time.
// 4) Code review: All lock guards scoped; checks for open state; safe arithmetic.
// 5) Secure code output: Final with 5 test cases.

#include <bits/stdc++.h>
#include <thread>
#include <mutex>

class Task181 {
public:
    class Bank {
        struct Account {
            long long id;
            long long balance;
            bool open;
            std::mutex mtx;
            Account(long long i, long long b) : id(i), balance(b), open(true) {}
        };

        std::unordered_map<long long, std::shared_ptr<Account>> accounts;
        std::mutex mapMtx;
        long long nextId = 1000;

        std::shared_ptr<Account> getAccount(long long id) {
            std::lock_guard<std::mutex> g(mapMtx);
            auto it = accounts.find(id);
            if (it == accounts.end()) return nullptr;
            return it->second;
        }

    public:
        long long openAccount(long long initialCents) {
            if (initialCents < 0) return -1;
            std::lock_guard<std::mutex> g(mapMtx);
            long long id = nextId++;
            accounts.emplace(id, std::make_shared<Account>(id, initialCents));
            return id;
        }

        bool closeAccount(long long id) {
            auto acc = getAccount(id);
            if (!acc) return false;
            std::lock_guard<std::mutex> g(acc->mtx);
            if (!acc->open) return false;
            if (acc->balance != 0) return false;
            acc->open = false;
            return true;
        }

        bool deposit(long long id, long long amountCents) {
            if (amountCents <= 0) return false;
            auto acc = getAccount(id);
            if (!acc) return false;
            std::lock_guard<std::mutex> g(acc->mtx);
            if (!acc->open) return false;
            if (acc->balance > LLONG_MAX - amountCents) return false;
            acc->balance += amountCents;
            return true;
        }

        bool withdraw(long long id, long long amountCents) {
            if (amountCents <= 0) return false;
            auto acc = getAccount(id);
            if (!acc) return false;
            std::lock_guard<std::mutex> g(acc->mtx);
            if (!acc->open) return false;
            if (acc->balance < amountCents) return false;
            acc->balance -= amountCents;
            return true;
        }

        long long getBalance(long long id) {
            auto acc = getAccount(id);
            if (!acc) return -1;
            std::lock_guard<std::mutex> g(acc->mtx);
            if (!acc->open) return -1;
            return acc->balance;
        }
    };

    static void runTests() {
        Bank bank;

        // Test 1
        long long acc1 = bank.openAccount(10000);
        bool d1 = bank.deposit(acc1, 5000);
        bool w1 = bank.withdraw(acc1, 3000);
        long long b1 = bank.getBalance(acc1);
        std::cout << "Test1 balance expected 12000: " << b1 << " ok=" << (d1 && w1 && b1 == 12000) << "\n";

        // Test 2: Concurrent deposits
        long long acc2 = bank.openAccount(0);
        int threads2 = 10, iters2 = 100;
        long long amount2 = 1000;
        std::vector<std::thread> th2;
        for (int t = 0; t < threads2; ++t) {
            th2.emplace_back([&]() {
                for (int i = 0; i < iters2; ++i) {
                    if (!bank.deposit(acc2, amount2)) {
                        throw std::runtime_error("Deposit failed unexpectedly");
                    }
                }
            });
        }
        for (auto &th : th2) th.join();
        long long expected2 = (long long)threads2 * iters2 * amount2;
        long long b2 = bank.getBalance(acc2);
        std::cout << "Test2 balance expected " << expected2 << ": " << b2 << " ok=" << (b2 == expected2) << "\n";

        // Test 3: Mixed concurrent ops
        long long acc3 = bank.openAccount(500000);
        int depThreads = 4, wthThreads = 3;
        int depIters = 100, wthIters = 80;
        long long depAmt = 1000, wthAmt = 1000;
        std::vector<std::thread> th3;
        for (int i = 0; i < depThreads; ++i) {
            th3.emplace_back([&]() {
                for (int k = 0; k < depIters; ++k) bank.deposit(acc3, depAmt);
            });
        }
        for (int i = 0; i < wthThreads; ++i) {
            th3.emplace_back([&]() {
                for (int k = 0; k < wthIters; ++k) bank.withdraw(acc3, wthAmt);
            });
        }
        for (auto &th : th3) th.join();
        long long expected3 = 500000 + depThreads * 1LL * depIters * depAmt - wthThreads * 1LL * wthIters * wthAmt;
        long long b3 = bank.getBalance(acc3);
        std::cout << "Test3 balance expected " << expected3 << ": " << b3 << " ok=" << (b3 == expected3) << "\n";

        // Test 4: Close account and operations must fail
        long long acc4 = bank.openAccount(0);
        bool close4 = bank.closeAccount(acc4);
        bool dep4 = bank.deposit(acc4, 1000);
        bool wth4 = bank.withdraw(acc4, 1000);
        long long b4 = bank.getBalance(acc4);
        std::cout << "Test4 close ok=" << close4
                  << " depositFail=" << (!dep4)
                  << " withdrawFail=" << (!wth4)
                  << " balanceIsNeg1=" << (b4 == -1) << "\n";

        // Test 5: Non-existent account
        long long fake = 9'999'999;
        bool dep5 = bank.deposit(fake, 100);
        bool wth5 = bank.withdraw(fake, 100);
        bool close5 = bank.closeAccount(fake);
        long long b5 = bank.getBalance(fake);
        std::cout << "Test5 all fail: " << (!dep5 && !wth5 && !close5 && b5 == -1) << "\n";
    }
};

int main() {
    Task181::runTests();
    return 0;
}