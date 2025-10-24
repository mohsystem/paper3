import java.util.concurrent.*;
import java.util.concurrent.locks.*;
import java.util.*;
import java.util.concurrent.atomic.AtomicLong;

public class Task181 {
    public static class Bank {
        private static class Account {
            private final ReentrantLock lock = new ReentrantLock();
            private long balance;
            private boolean closed;

            Account(long initial) {
                this.balance = initial;
                this.closed = false;
            }
        }

        private final ConcurrentHashMap<Long, Account> accounts = new ConcurrentHashMap<>();
        private final AtomicLong nextId = new AtomicLong(1);

        public long openAccount(long initialCents) {
            if (initialCents < 0) {
                return -1;
            }
            long id = nextId.getAndIncrement();
            accounts.put(id, new Account(initialCents));
            return id;
        }

        public boolean closeAccount(long accountId) {
            Account acc = accounts.get(accountId);
            if (acc == null) return false;
            acc.lock.lock();
            try {
                if (acc.closed) return false;
                acc.closed = true;
                return true;
            } finally {
                acc.lock.unlock();
            }
        }

        public boolean deposit(long accountId, long cents) {
            if (cents <= 0) return false;
            Account acc = accounts.get(accountId);
            if (acc == null) return false;
            acc.lock.lock();
            try {
                if (acc.closed) return false;
                if (acc.balance > Long.MAX_VALUE - cents) return false; // overflow check
                acc.balance += cents;
                return true;
            } finally {
                acc.lock.unlock();
            }
        }

        public boolean withdraw(long accountId, long cents) {
            if (cents <= 0) return false;
            Account acc = accounts.get(accountId);
            if (acc == null) return false;
            acc.lock.lock();
            try {
                if (acc.closed) return false;
                if (cents > acc.balance) return false; // insufficient funds
                acc.balance -= cents;
                return true;
            } finally {
                acc.lock.unlock();
            }
        }

        public Long getBalance(long accountId) {
            Account acc = accounts.get(accountId);
            if (acc == null) return null;
            acc.lock.lock();
            try {
                if (acc.closed) return null;
                return acc.balance;
            } finally {
                acc.lock.unlock();
            }
        }
    }

    private static void runConcurrent(Runnable r, int threads) throws InterruptedException {
        Thread[] ts = new Thread[threads];
        CountDownLatch start = new CountDownLatch(1);
        CountDownLatch done = new CountDownLatch(threads);
        for (int i = 0; i < threads; i++) {
            ts[i] = new Thread(() -> {
                try {
                    start.await();
                    r.run();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                } finally {
                    done.countDown();
                }
            });
            ts[i].start();
        }
        start.countDown();
        done.await();
    }

    public static void main(String[] args) throws Exception {
        Bank bank = new Bank();

        // Test 1: Basic open, deposit, withdraw, balance
        long a1 = bank.openAccount(10_000);
        boolean d1 = bank.deposit(a1, 5_000);
        boolean w1 = bank.withdraw(a1, 2_000);
        Long b1 = bank.getBalance(a1);
        System.out.println("Test1: " + (d1 && w1 && b1 != null && b1 == 13_000));

        // Test 2: Concurrent deposits
        long a2 = bank.openAccount(0);
        int threads2 = 10;
        int perThreadOps2 = 1000;
        long perOp2 = 1_000;
        runConcurrent(() -> {
            for (int i = 0; i < perThreadOps2; i++) {
                bank.deposit(a2, perOp2);
            }
        }, threads2);
        Long b2 = bank.getBalance(a2);
        long expected2 = (long) threads2 * perThreadOps2 * perOp2;
        System.out.println("Test2: " + (b2 != null && b2 == expected2));

        // Test 3: Mixed concurrent deposits and withdrawals
        long a3 = bank.openAccount(1_000_000);
        int threads3 = 8;
        int perThreadOps3 = 5000;
        runConcurrent(() -> {
            for (int i = 0; i < perThreadOps3; i++) {
                bank.deposit(a3, 100);
                bank.withdraw(a3, 100);
            }
        }, threads3);
        Long b3 = bank.getBalance(a3);
        System.out.println("Test3: " + (b3 != null && b3 == 1_000_000));

        // Test 4: Close account then operations should fail gracefully
        long a4 = bank.openAccount(50_000);
        boolean c4 = bank.closeAccount(a4);
        boolean d4 = bank.deposit(a4, 1_000);
        boolean w4 = bank.withdraw(a4, 1_000);
        Long b4 = bank.getBalance(a4);
        System.out.println("Test4: " + (c4 && !d4 && !w4 && b4 == null));

        // Test 5: Insufficient funds and invalid inputs
        long a5 = bank.openAccount(5_000);
        boolean w5a = bank.withdraw(a5, 10_000); // should fail
        boolean d5a = bank.deposit(a5, -100);    // invalid
        boolean w5b = bank.withdraw(a5, 0);      // invalid
        Long b5 = bank.getBalance(a5);
        System.out.println("Test5: " + (!w5a && !d5a && !w5b && b5 != null && b5 == 5_000));
    }
}