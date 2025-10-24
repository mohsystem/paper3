
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantLock;

class Task181 {
    private static class BankAccount {
        private double balance;
        private boolean closed;
        private final ReentrantLock lock;

        public BankAccount(double initialBalance) {
            this.balance = initialBalance;
            this.closed = false;
            this.lock = new ReentrantLock();
        }

        public boolean deposit(double amount) {
            lock.lock();
            try {
                if (closed) {
                    return false;
                }
                if (amount <= 0) {
                    return false;
                }
                balance += amount;
                return true;
            } finally {
                lock.unlock();
            }
        }

        public boolean withdraw(double amount) {
            lock.lock();
            try {
                if (closed) {
                    return false;
                }
                if (amount <= 0 || amount > balance) {
                    return false;
                }
                balance -= amount;
                return true;
            } finally {
                lock.unlock();
            }
        }

        public double getBalance() {
            lock.lock();
            try {
                if (closed) {
                    return -1;
                }
                return balance;
            } finally {
                lock.unlock();
            }
        }

        public boolean close() {
            lock.lock();
            try {
                if (closed) {
                    return false;
                }
                closed = true;
                return true;
            } finally {
                lock.unlock();
            }
        }

        public boolean isClosed() {
            lock.lock();
            try {
                return closed;
            } finally {
                lock.unlock();
            }
        }
    }

    private final ConcurrentHashMap<String, BankAccount> accounts;

    public Task181() {
        this.accounts = new ConcurrentHashMap<>();
    }

    public boolean openAccount(String accountId, double initialBalance) {
        if (accountId == null || initialBalance < 0) {
            return false;
        }
        BankAccount newAccount = new BankAccount(initialBalance);
        return accounts.putIfAbsent(accountId, newAccount) == null;
    }

    public boolean closeAccount(String accountId) {
        BankAccount account = accounts.get(accountId);
        if (account == null) {
            return false;
        }
        return account.close();
    }

    public boolean deposit(String accountId, double amount) {
        BankAccount account = accounts.get(accountId);
        if (account == null) {
            return false;
        }
        return account.deposit(amount);
    }

    public boolean withdraw(String accountId, double amount) {
        BankAccount account = accounts.get(accountId);
        if (account == null) {
            return false;
        }
        return account.withdraw(amount);
    }

    public double getBalance(String accountId) {
        BankAccount account = accounts.get(accountId);
        if (account == null) {
            return -1;
        }
        return account.getBalance();
    }

    public static void main(String[] args) throws InterruptedException {
        Task181 bank = new Task181();

        // Test Case 1: Open account and deposit
        System.out.println("Test Case 1: Open account and deposit");
        bank.openAccount("ACC001", 1000.0);
        bank.deposit("ACC001", 500.0);
        System.out.println("Balance: " + bank.getBalance("ACC001")); // Expected: 1500.0

        // Test Case 2: Withdraw from account
        System.out.println("\\nTest Case 2: Withdraw from account");
        bank.withdraw("ACC001", 300.0);
        System.out.println("Balance: " + bank.getBalance("ACC001")); // Expected: 1200.0

        // Test Case 3: Close account and try operations
        System.out.println("\\nTest Case 3: Close account and try operations");
        bank.closeAccount("ACC001");
        System.out.println("Deposit after close: " + bank.deposit("ACC001", 100.0)); // Expected: false
        System.out.println("Balance after close: " + bank.getBalance("ACC001")); // Expected: -1

        // Test Case 4: Concurrent operations
        System.out.println("\\nTest Case 4: Concurrent operations");
        bank.openAccount("ACC002", 5000.0);
        Thread t1 = new Thread(() -> {
            for (int i = 0; i < 100; i++) {
                bank.deposit("ACC002", 10.0);
            }
        });
        Thread t2 = new Thread(() -> {
            for (int i = 0; i < 100; i++) {
                bank.withdraw("ACC002", 5.0);
            }
        });
        t1.start();
        t2.start();
        t1.join();
        t2.join();
        System.out.println("Balance after concurrent ops: " + bank.getBalance("ACC002")); // Expected: 5500.0

        // Test Case 5: Invalid operations
        System.out.println("\\nTest Case 5: Invalid operations");
        System.out.println("Open duplicate account: " + bank.openAccount("ACC002", 1000.0)); // Expected: false
        System.out.println("Withdraw from non-existent: " + bank.withdraw("ACC999", 100.0)); // Expected: false
        System.out.println("Overdraw: " + bank.withdraw("ACC002", 10000.0)); // Expected: false
    }
}
