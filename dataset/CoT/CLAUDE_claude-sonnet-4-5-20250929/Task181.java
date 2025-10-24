
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.ConcurrentHashMap;

class Task181 {
    private static class BankAccount {
        private final String accountId;
        private long balance; // Using long to avoid overflow
        private boolean isClosed;
        private final ReentrantLock lock;

        public BankAccount(String accountId, long initialBalance) {
            if (initialBalance < 0) {
                throw new IllegalArgumentException("Initial balance cannot be negative");
            }
            this.accountId = accountId;
            this.balance = initialBalance;
            this.isClosed = false;
            this.lock = new ReentrantLock();
        }

        public boolean deposit(long amount) {
            if (amount <= 0) {
                return false;
            }
            
            lock.lock();
            try {
                if (isClosed) {
                    return false;
                }
                // Check for overflow
                if (balance > Long.MAX_VALUE - amount) {
                    return false;
                }
                balance += amount;
                return true;
            } finally {
                lock.unlock();
            }
        }

        public boolean withdraw(long amount) {
            if (amount <= 0) {
                return false;
            }
            
            lock.lock();
            try {
                if (isClosed) {
                    return false;
                }
                if (balance < amount) {
                    return false;
                }
                balance -= amount;
                return true;
            } finally {
                lock.unlock();
            }
        }

        public long getBalance() {
            lock.lock();
            try {
                return isClosed ? -1 : balance;
            } finally {
                lock.unlock();
            }
        }

        public boolean close() {
            lock.lock();
            try {
                if (isClosed) {
                    return false;
                }
                isClosed = true;
                return true;
            } finally {
                lock.unlock();
            }
        }

        public boolean isOpen() {
            lock.lock();
            try {
                return !isClosed;
            } finally {
                lock.unlock();
            }
        }
    }

    private static class BankingSystem {
        private final ConcurrentHashMap<String, BankAccount> accounts;

        public BankingSystem() {
            this.accounts = new ConcurrentHashMap<>();
        }

        public boolean openAccount(String accountId, long initialBalance) {
            if (accountId == null || accountId.trim().isEmpty()) {
                return false;
            }
            BankAccount newAccount = new BankAccount(accountId, initialBalance);
            return accounts.putIfAbsent(accountId, newAccount) == null;
        }

        public boolean closeAccount(String accountId) {
            BankAccount account = accounts.get(accountId);
            return account != null && account.close();
        }

        public boolean deposit(String accountId, long amount) {
            BankAccount account = accounts.get(accountId);
            return account != null && account.deposit(amount);
        }

        public boolean withdraw(String accountId, long amount) {
            BankAccount account = accounts.get(accountId);
            return account != null && account.withdraw(amount);
        }

        public long getBalance(String accountId) {
            BankAccount account = accounts.get(accountId);
            return account != null ? account.getBalance() : -1;
        }
    }

    public static void main(String[] args) {
        BankingSystem bank = new BankingSystem();

        // Test Case 1: Open account and deposit
        System.out.println("Test 1 - Open account and deposit:");
        bank.openAccount("ACC001", 1000);
        bank.deposit("ACC001", 500);
        System.out.println("Balance: " + bank.getBalance("ACC001")); // Expected: 1500

        // Test Case 2: Withdraw from account
        System.out.println("\\nTest 2 - Withdraw:");
        bank.withdraw("ACC001", 300);
        System.out.println("Balance: " + bank.getBalance("ACC001")); // Expected: 1200

        // Test Case 3: Close account and try operations
        System.out.println("\\nTest 3 - Close account:");
        bank.closeAccount("ACC001");
        boolean depositResult = bank.deposit("ACC001", 100);
        System.out.println("Deposit after close: " + depositResult); // Expected: false
        System.out.println("Balance after close: " + bank.getBalance("ACC001")); // Expected: -1

        // Test Case 4: Insufficient funds
        System.out.println("\\nTest 4 - Insufficient funds:");
        bank.openAccount("ACC002", 100);
        boolean withdrawResult = bank.withdraw("ACC002", 200);
        System.out.println("Withdraw result: " + withdrawResult); // Expected: false

        // Test Case 5: Concurrent operations
        System.out.println("\\nTest 5 - Concurrent operations:");
        bank.openAccount("ACC003", 1000);
        Thread t1 = new Thread(() -> {
            for (int i = 0; i < 100; i++) {
                bank.deposit("ACC003", 10);
            }
        });
        Thread t2 = new Thread(() -> {
            for (int i = 0; i < 100; i++) {
                bank.withdraw("ACC003", 5);
            }
        });
        t1.start();
        t2.start();
        try {
            t1.join();
            t2.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        System.out.println("Final balance: " + bank.getBalance("ACC003")); // Expected: 1500
    }
}
