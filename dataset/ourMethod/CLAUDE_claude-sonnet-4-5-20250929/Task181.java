
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.math.BigDecimal;
import java.util.Map;

public class Task181 {
    private static class BankAccount {
        private final String accountId;
        private BigDecimal balance;
        private boolean closed;
        private final ReentrantReadWriteLock lock;

        public BankAccount(String accountId, BigDecimal initialBalance) {
            if (accountId == null || accountId.trim().isEmpty()) {
                throw new IllegalArgumentException("Account ID cannot be null or empty");
            }
            if (initialBalance == null || initialBalance.compareTo(BigDecimal.ZERO) < 0) {
                throw new IllegalArgumentException("Initial balance cannot be null or negative");
            }
            this.accountId = accountId;
            this.balance = initialBalance;
            this.closed = false;
            this.lock = new ReentrantReadWriteLock(true);
        }

        public boolean deposit(BigDecimal amount) {
            if (amount == null || amount.compareTo(BigDecimal.ZERO) <= 0) {
                return false;
            }
            lock.writeLock().lock();
            try {
                if (closed) {
                    return false;
                }
                balance = balance.add(amount);
                return true;
            } finally {
                lock.writeLock().unlock();
            }
        }

        public boolean withdraw(BigDecimal amount) {
            if (amount == null || amount.compareTo(BigDecimal.ZERO) <= 0) {
                return false;
            }
            lock.writeLock().lock();
            try {
                if (closed) {
                    return false;
                }
                if (balance.compareTo(amount) < 0) {
                    return false;
                }
                balance = balance.subtract(amount);
                return true;
            } finally {
                lock.writeLock().unlock();
            }
        }

        public BigDecimal getBalance() {
            lock.readLock().lock();
            try {
                if (closed) {
                    return null;
                }
                return balance;
            } finally {
                lock.readLock().unlock();
            }
        }

        public boolean close() {
            lock.writeLock().lock();
            try {
                if (closed) {
                    return false;
                }
                closed = true;
                return true;
            } finally {
                lock.writeLock().unlock();
            }
        }

        public boolean isClosed() {
            lock.readLock().lock();
            try {
                return closed;
            } finally {
                lock.readLock().unlock();
            }
        }

        public String getAccountId() {
            return accountId;
        }
    }

    private static class BankingSystem {
        private final ConcurrentHashMap<String, BankAccount> accounts;

        public BankingSystem() {
            this.accounts = new ConcurrentHashMap<>();
        }

        public boolean openAccount(String accountId, BigDecimal initialBalance) {
            if (accountId == null || accountId.trim().isEmpty()) {
                return false;
            }
            if (initialBalance == null || initialBalance.compareTo(BigDecimal.ZERO) < 0) {
                return false;
            }
            try {
                BankAccount newAccount = new BankAccount(accountId, initialBalance);
                return accounts.putIfAbsent(accountId, newAccount) == null;
            } catch (IllegalArgumentException e) {
                return false;
            }
        }

        public boolean closeAccount(String accountId) {
            if (accountId == null || accountId.trim().isEmpty()) {
                return false;
            }
            BankAccount account = accounts.get(accountId);
            if (account == null) {
                return false;
            }
            return account.close();
        }

        public boolean deposit(String accountId, BigDecimal amount) {
            if (accountId == null || accountId.trim().isEmpty()) {
                return false;
            }
            BankAccount account = accounts.get(accountId);
            if (account == null) {
                return false;
            }
            return account.deposit(amount);
        }

        public boolean withdraw(String accountId, BigDecimal amount) {
            if (accountId == null || accountId.trim().isEmpty()) {
                return false;
            }
            BankAccount account = accounts.get(accountId);
            if (account == null) {
                return false;
            }
            return account.withdraw(amount);
        }

        public BigDecimal getBalance(String accountId) {
            if (accountId == null || accountId.trim().isEmpty()) {
                return null;
            }
            BankAccount account = accounts.get(accountId);
            if (account == null) {
                return null;
            }
            return account.getBalance();
        }
    }

    public static void main(String[] args) {
        BankingSystem bank = new BankingSystem();

        System.out.println("Test 1: Open account and check balance");
        boolean opened = bank.openAccount("ACC001", new BigDecimal("1000.00"));
        BigDecimal balance = bank.getBalance("ACC001");
        System.out.println("Account opened: " + opened + ", Balance: " + balance);

        System.out.println("\\nTest 2: Deposit money");
        boolean deposited = bank.deposit("ACC001", new BigDecimal("500.00"));
        balance = bank.getBalance("ACC001");
        System.out.println("Deposit successful: " + deposited + ", New balance: " + balance);

        System.out.println("\\nTest 3: Withdraw money");
        boolean withdrawn = bank.withdraw("ACC001", new BigDecimal("300.00"));
        balance = bank.getBalance("ACC001");
        System.out.println("Withdrawal successful: " + withdrawn + ", New balance: " + balance);

        System.out.println("\\nTest 4: Close account and try to deposit");
        boolean closed = bank.closeAccount("ACC001");
        boolean depositAfterClose = bank.deposit("ACC001", new BigDecimal("100.00"));
        System.out.println("Account closed: " + closed + ", Deposit after close: " + depositAfterClose);

        System.out.println("\\nTest 5: Concurrent operations");
        bank.openAccount("ACC002", new BigDecimal("2000.00"));
        Thread t1 = new Thread(() -> {
            for (int i = 0; i < 5; i++) {
                bank.deposit("ACC002", new BigDecimal("10.00"));
            }
        });
        Thread t2 = new Thread(() -> {
            for (int i = 0; i < 5; i++) {
                bank.withdraw("ACC002", new BigDecimal("5.00"));
            }
        });
        t1.start();
        t2.start();
        try {
            t1.join();
            t2.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        balance = bank.getBalance("ACC002");
        System.out.println("Final balance after concurrent operations: " + balance);
    }
}
