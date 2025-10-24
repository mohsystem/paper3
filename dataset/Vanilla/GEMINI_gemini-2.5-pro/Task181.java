import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class Task181 {

    public static class BankAccount {
        private long balance;
        private boolean isOpen;

        public BankAccount() {
            // Account is created in a closed state, must be opened to be used.
            this.balance = 0;
            this.isOpen = false;
        }

        public synchronized void open(long initialDeposit) {
            if (this.isOpen) {
                // To prevent re-opening, we could throw an exception or just return.
                // For this example, we'll allow re-opening to reset the account.
            }
            if (initialDeposit < 0) {
                throw new IllegalArgumentException("Initial deposit cannot be negative.");
            }
            this.balance = initialDeposit;
            this.isOpen = true;
        }

        public synchronized void close() {
            this.isOpen = false;
            // Optionally, we can also set balance to 0.
            // this.balance = 0;
        }

        public synchronized long getBalance() throws IllegalStateException {
            if (!this.isOpen) {
                throw new IllegalStateException("Account is closed.");
            }
            return this.balance;
        }

        public synchronized void deposit(long amount) throws IllegalStateException {
            if (!this.isOpen) {
                throw new IllegalStateException("Account is closed.");
            }
            if (amount <= 0) {
                throw new IllegalArgumentException("Deposit amount must be positive.");
            }
            this.balance += amount;
        }

        public synchronized void withdraw(long amount) throws IllegalStateException, IllegalArgumentException {
            if (!this.isOpen) {
                throw new IllegalStateException("Account is closed.");
            }
            if (amount <= 0) {
                throw new IllegalArgumentException("Withdrawal amount must be positive.");
            }
            if (this.balance < amount) {
                throw new IllegalArgumentException("Insufficient funds.");
            }
            this.balance -= amount;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Bank Account Tests ---");

        // Test Case 1: Simple Deposit/Withdraw
        System.out.println("\n--- Test Case 1: Simple Transactions ---");
        try {
            BankAccount acc1 = new BankAccount();
            acc1.open(100);
            System.out.println("Opened with balance: " + acc1.getBalance());
            acc1.deposit(50);
            System.out.println("After deposit of 50, balance: " + acc1.getBalance());
            acc1.withdraw(30);
            System.out.println("After withdrawal of 30, balance: " + acc1.getBalance());
            if (acc1.getBalance() == 120) {
                System.out.println("Test Case 1 PASSED");
            } else {
                System.out.println("Test Case 1 FAILED");
            }

            // Test Case 2: Insufficient Funds
            System.out.println("\n--- Test Case 2: Insufficient Funds ---");
            try {
                System.out.println("Attempting to withdraw 200 from 120...");
                acc1.withdraw(200);
                System.out.println("Test Case 2 FAILED");
            } catch (IllegalArgumentException e) {
                System.out.println("Caught expected exception: " + e.getMessage());
                System.out.println("Test Case 2 PASSED");
            }

            // Test Case 3: Closed Account Operations
            System.out.println("\n--- Test Case 3: Closed Account ---");
            acc1.close();
            System.out.println("Account closed.");
            try {
                acc1.getBalance();
                System.out.println("Test Case 3 FAILED (getBalance on closed account)");
            } catch (IllegalStateException e) {
                System.out.println("Caught expected exception on getBalance: " + e.getMessage());
                System.out.println("Test Case 3 PASSED");
            }
        } catch (Exception e) {
            System.out.println("An unexpected error occurred: " + e.getMessage());
        }

        // Test Case 4: Concurrent Transactions
        System.out.println("\n--- Test Case 4: Concurrent Transactions ---");
        try {
            final BankAccount sharedAccount = new BankAccount();
            sharedAccount.open(10000);
            final int transactions = 1000;
            final int threads = 10;
            
            List<Thread> threadList = new ArrayList<>();
            for (int i = 0; i < threads; i++) {
                threadList.add(new Thread(() -> {
                    for (int j = 0; j < transactions; j++) sharedAccount.deposit(10);
                }));
                threadList.add(new Thread(() -> {
                    for (int j = 0; j < transactions; j++) sharedAccount.withdraw(10);
                }));
            }
            for (Thread t : threadList) t.start();
            for (Thread t : threadList) t.join();

            long finalBalance = sharedAccount.getBalance();
            System.out.println("Initial Balance: 10000");
            System.out.println("Final Balance after concurrent transactions: " + finalBalance);
            if (finalBalance == 10000) {
                System.out.println("Test Case 4 PASSED");
            } else {
                System.out.println("Test Case 4 FAILED");
            }
        } catch (Exception e) {
            System.out.println("An unexpected error occurred in concurrency test: " + e.getMessage());
        }
        
        // Test Case 5: Zero/Negative Amount
        System.out.println("\n--- Test Case 5: Invalid Amount ---");
        try {
            BankAccount acc5 = new BankAccount();
            acc5.open(100);
            try {
                System.out.println("Attempting to deposit -50...");
                acc5.deposit(-50);
                System.out.println("Test Case 5 FAILED (negative deposit)");
            } catch (IllegalArgumentException e) {
                System.out.println("Caught expected exception: " + e.getMessage());
            }
            try {
                System.out.println("Attempting to withdraw 0...");
                acc5.withdraw(0);
                System.out.println("Test Case 5 FAILED (zero withdrawal)");
            } catch (IllegalArgumentException e) {
                System.out.println("Caught expected exception: " + e.getMessage());
                System.out.println("Test Case 5 PASSED");
            }
        } catch (Exception e) {
            System.out.println("An unexpected error occurred: " + e.getMessage());
        }
    }
}