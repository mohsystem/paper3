import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.*;
import java.util.concurrent.locks.ReentrantLock;

public class Task181 {
    public static class BankAccount {
        private final ReentrantLock lock = new ReentrantLock();
        private boolean open = false;
        private long balance = 0;

        public void open() {
            lock.lock();
            try {
                if (open) throw new IllegalStateException("Account already open");
                open = true;
                balance = 0;
            } finally {
                lock.unlock();
            }
        }

        public void close() {
            lock.lock();
            try {
                if (!open) throw new IllegalStateException("Account already closed");
                if (balance != 0) throw new IllegalStateException("Cannot close account with non-zero balance");
                open = false;
            } finally {
                lock.unlock();
            }
        }

        public long deposit(long amount) {
            if (amount < 0) throw new IllegalArgumentException("Negative deposit");
            lock.lock();
            try {
                ensureOpen();
                balance += amount;
                return balance;
            } finally {
                lock.unlock();
            }
        }

        public long withdraw(long amount) {
            if (amount < 0) throw new IllegalArgumentException("Negative withdraw");
            lock.lock();
            try {
                ensureOpen();
                if (amount > balance) throw new IllegalStateException("Insufficient funds");
                balance -= amount;
                return balance;
            } finally {
                lock.unlock();
            }
        }

        public long getBalance() {
            lock.lock();
            try {
                ensureOpen();
                return balance;
            } finally {
                lock.unlock();
            }
        }

        private void ensureOpen() {
            if (!open) throw new IllegalStateException("Account is closed");
        }
    }

    public static void main(String[] args) throws Exception {
        // Test 1: Simple open, deposit, withdraw
        BankAccount a1 = new BankAccount();
        a1.open();
        a1.deposit(200);
        a1.withdraw(50);
        System.out.println("Test1 balance (expected 150): " + a1.getBalance());

        // Test 2: Concurrent deposits
        BankAccount a2 = new BankAccount();
        a2.open();
        int threads2 = 8;
        int depositsPerThread2 = 10000;
        ExecutorService es2 = Executors.newFixedThreadPool(threads2);
        for (int i = 0; i < threads2; i++) {
            es2.submit(() -> {
                for (int j = 0; j < depositsPerThread2; j++) {
                    a2.deposit(1);
                }
            });
        }
        es2.shutdown();
        es2.awaitTermination(1, TimeUnit.MINUTES);
        System.out.println("Test2 balance (expected " + (threads2 * depositsPerThread2) + "): " + a2.getBalance());

        // Test 3: Over-withdraw should fail gracefully
        BankAccount a3 = new BankAccount();
        a3.open();
        a3.deposit(50);
        try {
            a3.withdraw(100);
            System.out.println("Test3 unexpected success");
        } catch (Exception ex) {
            System.out.println("Test3 over-withdraw failed as expected: " + ex.getMessage());
        }

        // Test 4: Close then attempt operation should fail
        BankAccount a4 = new BankAccount();
        a4.open();
        a4.deposit(75);
        a4.withdraw(75);
        a4.close();
        try {
            a4.deposit(10);
            System.out.println("Test4 unexpected success");
        } catch (Exception ex) {
            System.out.println("Test4 operation on closed account failed as expected: " + ex.getMessage());
        }

        // Test 5: Mixed concurrent operations
        BankAccount a5 = new BankAccount();
        a5.open();
        a5.deposit(1000);
        int withdrawThreads = 5;
        int withdrawsPerThread = 100; // total 500
        int depositThreads = 3;
        int depositsPerThread = 100; // total 300
        List<Callable<Void>> tasks = new ArrayList<>();
        for (int i = 0; i < withdrawThreads; i++) {
            tasks.add(() -> {
                for (int j = 0; j < withdrawsPerThread; j++) {
                    try {
                        a5.withdraw(1);
                    } catch (IllegalStateException ex) {
                        // insufficient funds or closed - ignore for this test if happens
                    }
                }
                return null;
            });
        }
        for (int i = 0; i < depositThreads; i++) {
            tasks.add(() -> {
                for (int j = 0; j < depositsPerThread; j++) {
                    a5.deposit(1);
                }
                return null;
            });
        }
        ExecutorService es5 = Executors.newFixedThreadPool(withdrawThreads + depositThreads);
        for (Callable<Void> t : tasks) es5.submit(t);
        es5.shutdown();
        es5.awaitTermination(1, TimeUnit.MINUTES);
        System.out.println("Test5 balance (expected 800): " + a5.getBalance());
    }
}