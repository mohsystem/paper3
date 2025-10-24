import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.locks.ReentrantLock;

public class Task181 {

    public static class Account {
        private double balance;
        private volatile boolean isOpen;
        private final ReentrantLock lock = new ReentrantLock();

        public Account(double initialBalance) {
            if (initialBalance < 0) {
                throw new IllegalArgumentException("Initial balance cannot be negative.");
            }
            this.balance = initialBalance;
            this.isOpen = true;
        }

        public void deposit(double amount) {
            if (amount <= 0) {
                throw new IllegalArgumentException("Deposit amount must be positive.");
            }
            lock.lock();
            try {
                if (!isOpen) {
                    throw new IllegalStateException("Account is closed.");
                }
                balance += amount;
            } finally {
                lock.unlock();
            }
        }

        public void withdraw(double amount) {
            if (amount <= 0) {
                throw new IllegalArgumentException("Withdrawal amount must be positive.");
            }
            lock.lock();
            try {
                if (!isOpen) {
                    throw new IllegalStateException("Account is closed.");
                }
                if (balance < amount) {
                    throw new IllegalStateException("Insufficient funds.");
                }
                balance -= amount;
            } finally {
                lock.unlock();
            }
        }

        public double getBalance() {
            lock.lock();
            try {
                if (!isOpen) {
                    throw new IllegalStateException("Account is closed.");
                }
                return balance;
            } finally {
                lock.unlock();
            }
        }

        public void close() {
            lock.lock();
            try {
                this.isOpen = false;
            } finally {
                lock.unlock();
            }
        }
    }

    private final ConcurrentHashMap<Long, Account> accounts = new ConcurrentHashMap<>();
    private final AtomicLong nextAccountId = new AtomicLong(1);

    public long openAccount(double initialBalance) {
        long accountId = nextAccountId.getAndIncrement();
        Account account = new Account(initialBalance);
        accounts.put(accountId, account);
        return accountId;
    }

    private Account getAccount(long accountId) {
        Account account = accounts.get(accountId);
        if (account == null) {
            throw new IllegalArgumentException("Account not found: " + accountId);
        }
        return account;
    }

    public void deposit(long accountId, double amount) {
        getAccount(accountId).deposit(amount);
    }

    public void withdraw(long accountId, double amount) {
        getAccount(accountId).withdraw(amount);
    }

    public double getBalance(long accountId) {
        return getAccount(accountId).getBalance();
    }

    public void closeAccount(long accountId) {
        // The account is not removed from the map, just marked as closed.
        // This prevents the accountId from being reused and avoids some race conditions.
        getAccount(accountId).close();
    }

    public static void main(String[] args) {
        Task181 bank = new Task181();
        System.out.println("--- Bank System Test ---");

        // Test Case 1: Open an account and check initial balance
        System.out.println("\n--- Test Case 1: Open Account ---");
        long acc1 = bank.openAccount(100.0);
        System.out.println("Opened account " + acc1 + " with balance: " + bank.getBalance(acc1));

        // Test Case 2: Deposit money
        System.out.println("\n--- Test Case 2: Deposit ---");
        System.out.println("Depositing 50.0 into account " + acc1);
        bank.deposit(acc1, 50.0);
        System.out.println("New balance for account " + acc1 + ": " + bank.getBalance(acc1));

        // Test Case 3: Withdraw money
        System.out.println("\n--- Test Case 3: Withdraw ---");
        System.out.println("Withdrawing 30.0 from account " + acc1);
        bank.withdraw(acc1, 30.0);
        System.out.println("New balance for account " + acc1 + ": " + bank.getBalance(acc1));

        // Test Case 4: Attempt to withdraw more than available funds
        System.out.println("\n--- Test Case 4: Insufficient Funds ---");
        try {
            System.out.println("Attempting to withdraw 200.0 from account " + acc1);
            bank.withdraw(acc1, 200.0);
        } catch (IllegalStateException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
            System.out.println("Balance remains: " + bank.getBalance(acc1));
        }

        // Test Case 5: Close account and attempt a transaction
        System.out.println("\n--- Test Case 5: Closed Account ---");
        System.out.println("Closing account " + acc1);
        bank.closeAccount(acc1);
        try {
            System.out.println("Attempting to deposit 10.0 into closed account " + acc1);
            bank.deposit(acc1, 10.0);
        } catch (IllegalStateException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
        try {
            System.out.println("Attempting to get balance of closed account " + acc1);
            bank.getBalance(acc1);
        } catch (IllegalStateException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
    }
}