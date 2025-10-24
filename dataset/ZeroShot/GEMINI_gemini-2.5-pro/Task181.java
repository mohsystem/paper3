import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.ReentrantLock;

class BankAccount {
    private long balance;
    private volatile boolean isOpen;
    private final ReentrantLock lock = new ReentrantLock();

    public BankAccount(long initialBalance) {
        this.balance = initialBalance;
        this.isOpen = true;
    }

    public boolean deposit(long amount) {
        if (amount <= 0) {
            return false;
        }
        lock.lock();
        try {
            if (!isOpen) {
                return false;
            }
            // Check for potential overflow, though unlikely with long
            if (balance > Long.MAX_VALUE - amount) {
                // In a real system, throw an exception or handle this error
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
            if (!isOpen || balance < amount) {
                return false;
            }
            balance -= amount;
            return true;
        } finally {
            lock.unlock();
        }
    }

    public Long getBalance() {
        lock.lock();
        try {
            if (!isOpen) {
                return null; // Use null to indicate a closed account
            }
            return balance;
        } finally {
            lock.unlock();
        }
    }
    
    public boolean isOpen() {
        return this.isOpen;
    }

    public void close() {
        lock.lock();
        try {
            isOpen = false;
        } finally {
            lock.unlock();
        }
    }
}

public class Task181 {
    private static final ConcurrentHashMap<Integer, BankAccount> accounts = new ConcurrentHashMap<>();
    private static final AtomicInteger accountIdGenerator = new AtomicInteger(0);

    public static int openAccount(long initialBalance) {
        if (initialBalance < 0) {
            return -1; // Invalid initial balance
        }
        int accountId = accountIdGenerator.incrementAndGet();
        BankAccount newAccount = new BankAccount(initialBalance);
        accounts.put(accountId, newAccount);
        return accountId;
    }

    public static boolean closeAccount(int accountId) {
        BankAccount account = accounts.get(accountId);
        if (account != null) {
            account.close();
            // Optionally remove from map if it's guaranteed no more operations will be attempted
            // accounts.remove(accountId);
            return true;
        }
        return false;
    }

    public static boolean deposit(int accountId, long amount) {
        BankAccount account = accounts.get(accountId);
        if (account != null) {
            return account.deposit(amount);
        }
        return false;
    }

    public static boolean withdraw(int accountId, long amount) {
        BankAccount account = accounts.get(accountId);
        if (account != null) {
            return account.withdraw(amount);
        }
        return false;
    }

    public static Long getBalance(int accountId) {
        BankAccount account = accounts.get(accountId);
        if (account != null) {
            return account.getBalance();
        }
        return null; // Indicates account not found or closed
    }

    public static void main(String[] args) {
        System.out.println("--- Java Bank Test Cases ---");

        // Test Case 1: Simple deposit and withdrawal
        System.out.println("\n--- Test Case 1: Simple Operations ---");
        int acc1 = openAccount(100);
        System.out.println("Opened account " + acc1 + " with balance: " + getBalance(acc1));
        deposit(acc1, 50);
        System.out.println("Deposited 50. New balance: " + getBalance(acc1));
        withdraw(acc1, 30);
        System.out.println("Withdrew 30. Final balance: " + getBalance(acc1));

        // Test Case 2: Insufficient funds
        System.out.println("\n--- Test Case 2: Insufficient Funds ---");
        int acc2 = openAccount(50);
        System.out.println("Opened account " + acc2 + " with balance: " + getBalance(acc2));
        boolean success = withdraw(acc2, 100);
        System.out.println("Attempted to withdraw 100. Success: " + success);
        System.out.println("Final balance: " + getBalance(acc2));

        // Test Case 3: Operations on a closed account
        System.out.println("\n--- Test Case 3: Closed Account Operations ---");
        int acc3 = openAccount(200);
        System.out.println("Opened account " + acc3 + " with balance: " + getBalance(acc3));
        closeAccount(acc3);
        System.out.println("Account " + acc3 + " closed.");
        System.out.println("Attempting deposit of 50. Success: " + deposit(acc3, 50));
        System.out.println("Attempting withdrawal of 20. Success: " + withdraw(acc3, 20));
        System.out.println("Final balance: " + getBalance(acc3));

        // Test Case 4: Concurrent deposits
        System.out.println("\n--- Test Case 4: Concurrent Deposits ---");
        int acc4 = openAccount(0);
        System.out.println("Opened account " + acc4 + " with balance: " + getBalance(acc4));
        ExecutorService depositService = Executors.newFixedThreadPool(10);
        for (int i = 0; i < 10; i++) {
            depositService.submit(() -> deposit(acc4, 10));
        }
        depositService.shutdown();
        try {
            depositService.awaitTermination(5, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        System.out.println("After 10 threads each deposited 10, final balance: " + getBalance(acc4));

        // Test Case 5: Concurrent deposits and withdrawals
        System.out.println("\n--- Test Case 5: Concurrent Mix ---");
        int acc5 = openAccount(1000);
        System.out.println("Opened account " + acc5 + " with balance: " + getBalance(acc5));
        ExecutorService mixService = Executors.newFixedThreadPool(10);
        for (int i = 0; i < 5; i++) {
            mixService.submit(() -> deposit(acc5, 20)); // Total deposit: 100
            mixService.submit(() -> withdraw(acc5, 30)); // Total withdrawal: 150
        }
        mixService.shutdown();
        try {
            mixService.awaitTermination(5, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        // Expected: 1000 + (5*20) - (5*30) = 950
        System.out.println("After 5 deposits of 20 and 5 withdrawals of 30, final balance: " + getBalance(acc5));
    }
}