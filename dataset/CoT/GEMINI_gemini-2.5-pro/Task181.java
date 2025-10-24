import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;

class BankAccount {
    private final AtomicLong balance;
    private volatile boolean isOpen;
    private final Object lock = new Object();

    public BankAccount(long initialBalance) {
        if (initialBalance < 0) {
            throw new IllegalArgumentException("Initial balance cannot be negative.");
        }
        this.balance = new AtomicLong(initialBalance);
        this.isOpen = true;
    }

    public long getBalance() throws IllegalStateException {
        if (!isOpen) {
            throw new IllegalStateException("Account is closed.");
        }
        return balance.get();
    }

    public void deposit(long amount) throws IllegalStateException {
        if (amount <= 0) {
            throw new IllegalArgumentException("Deposit amount must be positive.");
        }
        synchronized (lock) {
            if (!isOpen) {
                throw new IllegalStateException("Account is closed.");
            }
            balance.addAndGet(amount);
        }
    }

    public void withdraw(long amount) throws IllegalStateException {
        if (amount <= 0) {
            throw new IllegalArgumentException("Withdrawal amount must be positive.");
        }
        synchronized (lock) {
            if (!isOpen) {
                throw new IllegalStateException("Account is closed.");
            }
            if (balance.get() < amount) {
                throw new IllegalStateException("Insufficient funds.");
            }
            balance.addAndGet(-amount);
        }
    }

    public void closeAccount() {
        // No need to synchronize setting a volatile boolean if it's a one-way switch
        this.isOpen = false;
    }
}

public class Task181 {
    public static void main(String[] args) throws InterruptedException {
        // Test Case 1: Basic Operations
        System.out.println("--- Test Case 1: Basic Operations ---");
        BankAccount account1 = new BankAccount(1000);
        try {
            System.out.println("Initial Balance: " + account1.getBalance());
            account1.deposit(500);
            System.out.println("Balance after deposit: " + account1.getBalance());
            account1.withdraw(200);
            System.out.println("Balance after withdrawal: " + account1.getBalance());
        } catch (IllegalStateException e) {
            System.out.println("Error: " + e.getMessage());
        }

        // Test Case 2: Insufficient Funds
        System.out.println("\n--- Test Case 2: Insufficient Funds ---");
        try {
            System.out.println("Current Balance: " + account1.getBalance());
            System.out.println("Attempting to withdraw 2000...");
            account1.withdraw(2000);
        } catch (IllegalStateException e) {
            System.out.println("Caught expected error: " + e.getMessage());
        }

        // Test Case 3: Operations on a closed account
        System.out.println("\n--- Test Case 3: Operations on Closed Account ---");
        account1.closeAccount();
        System.out.println("Account closed.");
        try {
            account1.deposit(100);
        } catch (IllegalStateException e) {
            System.out.println("Caught expected error on deposit: " + e.getMessage());
        }
        try {
            account1.withdraw(50);
        } catch (IllegalStateException e) {
            System.out.println("Caught expected error on withdraw: " + e.getMessage());
        }
        try {
            account1.getBalance();
        } catch (IllegalStateException e) {
            System.out.println("Caught expected error on getBalance: " + e.getMessage());
        }

        // Test Case 4: Concurrent Transactions
        System.out.println("\n--- Test Case 4: Concurrent Transactions ---");
        BankAccount account2 = new BankAccount(10000);
        ExecutorService service = Executors.newFixedThreadPool(10);
        for (int i = 0; i < 1000; i++) {
            service.submit(() -> {
                try {
                    account2.deposit(10);
                } catch (IllegalStateException e) {
                    System.out.println(e.getMessage());
                }
            });
            service.submit(() -> {
                try {
                    account2.withdraw(5);
                } catch (IllegalStateException e) {
                    System.out.println(e.getMessage());
                }
            });
        }
        service.shutdown();
        service.awaitTermination(5, TimeUnit.SECONDS);
        System.out.println("Initial concurrent balance: 10000");
        System.out.println("After 1000 deposits of 10 and 1000 withdrawals of 5...");
        // Expected: 10000 + (1000 * 10) - (1000 * 5) = 15000
        System.out.println("Final concurrent balance: " + account2.getBalance());

        // Test Case 5: Opening with negative balance
        System.out.println("\n--- Test Case 5: Opening with Negative Balance ---");
        try {
            new BankAccount(-100);
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected error: " + e.getMessage());
        }
    }
}