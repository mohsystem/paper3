// Chain-of-Through process in code comments:
// 1) Problem understanding: Implement a thread-safe bank with open/close/deposit/withdraw/getBalance.
// 2) Security requirements: Avoid race conditions, validate inputs, prevent overflows, handle closed/non-existent accounts gracefully.
// 3) Secure coding generation: Use fine-grained locks per account; atomic id generation; bounds checks.
// 4) Code review: Locks always released; no deadlocks; checks for closed state; overflow/underflow checks.
// 5) Secure code output: Final code below with 5 test cases.

import java.util.concurrent.*;
import java.util.*;
import java.util.concurrent.locks.*;

public class Task181 {

    // Thread-safe bank implementation using per-account locks and a concurrent map
    public static final class Bank {
        private static final class Account {
            final long id;
            private long balanceCents;
            private boolean open;
            private final ReentrantLock lock = new ReentrantLock(true);

            Account(long id, long initial) {
                this.id = id;
                this.balanceCents = initial;
                this.open = true;
            }

            void lock() { lock.lock(); }
            void unlock() { lock.unlock(); }
            boolean isOpen() { return open; }
            boolean closeIfZeroBalance() {
                if (!open) return false;
                if (balanceCents != 0) return false;
                open = false;
                return true;
            }
            boolean deposit(long amount) {
                if (!open) return false;
                if (amount <= 0) return false;
                if (Long.MAX_VALUE - balanceCents < amount) return false; // overflow guard
                balanceCents += amount;
                return true;
            }
            boolean withdraw(long amount) {
                if (!open) return false;
                if (amount <= 0) return false;
                if (balanceCents < amount) return false;
                balanceCents -= amount;
                return true;
            }
            long getBalanceUnsafe() { return balanceCents; }
        }

        private final ConcurrentHashMap<Long, Account> accounts = new ConcurrentHashMap<>();
        private final AtomicLong nextId = new AtomicLong(1000L);

        public long openAccount(long initialCents) {
            if (initialCents < 0) return -1L;
            long id = nextId.getAndIncrement();
            Account acc = new Account(id, initialCents);
            accounts.put(id, acc);
            return id;
        }

        public boolean closeAccount(long id) {
            Account acc = accounts.get(id);
            if (acc == null) return false;
            acc.lock();
            try {
                return acc.closeIfZeroBalance();
            } finally {
                acc.unlock();
            }
        }

        public boolean deposit(long id, long amountCents) {
            Account acc = accounts.get(id);
            if (acc == null) return false;
            acc.lock();
            try {
                return acc.deposit(amountCents);
            } finally {
                acc.unlock();
            }
        }

        public boolean withdraw(long id, long amountCents) {
            Account acc = accounts.get(id);
            if (acc == null) return false;
            acc.lock();
            try {
                return acc.withdraw(amountCents);
            } finally {
                acc.unlock();
            }
        }

        public long getBalance(long id) {
            Account acc = accounts.get(id);
            if (acc == null) return -1L;
            acc.lock();
            try {
                if (!acc.isOpen()) return -1L;
                return acc.getBalanceUnsafe();
            } finally {
                acc.unlock();
            }
        }
    }

    // Main with 5 test cases
    public static void main(String[] args) throws Exception {
        Bank bank = new Bank();

        // Test 1: Basic open, deposit, withdraw, check balance
        long acc1 = bank.openAccount(10_000);
        boolean d1 = bank.deposit(acc1, 5_000);
        boolean w1 = bank.withdraw(acc1, 3_000);
        long b1 = bank.getBalance(acc1);
        System.out.println("Test1 balance expected 12000: " + b1 + " ok=" + (d1 && w1 && b1 == 12_000));

        // Test 2: Concurrent deposits
        long acc2 = bank.openAccount(0);
        int threads2 = 10;
        int iters2 = 100;
        long amount2 = 1_000;
        ExecutorService es2 = Executors.newFixedThreadPool(threads2);
        CountDownLatch latch2 = new CountDownLatch(threads2);
        for (int t = 0; t < threads2; t++) {
            es2.execute(() -> {
                for (int i = 0; i < iters2; i++) {
                    if (!bank.deposit(acc2, amount2)) {
                        // Should not fail
                        throw new RuntimeException("Deposit failed unexpectedly");
                    }
                }
                latch2.countDown();
            });
        }
        latch2.await();
        es2.shutdown();
        long expected2 = threads2 * (long) iters2 * amount2;
        long b2 = bank.getBalance(acc2);
        System.out.println("Test2 balance expected " + expected2 + ": " + b2 + " ok=" + (b2 == expected2));

        // Test 3: Mixed concurrent deposit and withdraw
        long acc3 = bank.openAccount(500_000);
        int depThreads = 4, wthThreads = 3;
        int depIters = 100, wthIters = 80;
        long depAmt = 1_000, wthAmt = 1_000;
        ExecutorService es3 = Executors.newFixedThreadPool(depThreads + wthThreads);
        CountDownLatch latch3 = new CountDownLatch(depThreads + wthThreads);
        for (int t = 0; t < depThreads; t++) {
            es3.execute(() -> {
                for (int i = 0; i < depIters; i++) bank.deposit(acc3, depAmt);
                latch3.countDown();
            });
        }
        for (int t = 0; t < wthThreads; t++) {
            es3.execute(() -> {
                for (int i = 0; i < wthIters; i++) bank.withdraw(acc3, wthAmt);
                latch3.countDown();
            });
        }
        latch3.await();
        es3.shutdown();
        long expected3 = 500_000 + depThreads * (long) depIters * depAmt - wthThreads * (long) wthIters * wthAmt;
        long b3 = bank.getBalance(acc3);
        System.out.println("Test3 balance expected " + expected3 + ": " + b3 + " ok=" + (b3 == expected3));

        // Test 4: Close account and operations must fail
        long acc4 = bank.openAccount(0);
        boolean close4a = bank.closeAccount(acc4);
        boolean dep4 = bank.deposit(acc4, 1_000);
        boolean wth4 = bank.withdraw(acc4, 1_000);
        long b4 = bank.getBalance(acc4); // returns -1 when closed
        System.out.println("Test4 close ok=" + close4a + " depositFail=" + (!dep4) + " withdrawFail=" + (!wth4) + " balanceIsNeg1=" + (b4 == -1));

        // Test 5: Non-existent account operations
        long fakeId = 9_999_999L;
        boolean dep5 = bank.deposit(fakeId, 100);
        boolean wth5 = bank.withdraw(fakeId, 100);
        boolean close5 = bank.closeAccount(fakeId);
        long b5 = bank.getBalance(fakeId);
        System.out.println("Test5 all fail: " + (!dep5 && !wth5 && !close5 && b5 == -1));
    }
}