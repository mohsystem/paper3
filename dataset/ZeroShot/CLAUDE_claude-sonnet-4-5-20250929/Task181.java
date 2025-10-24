
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.math.BigDecimal;

class Task181 {
    private static class Account {
        private BigDecimal balance;
        private boolean closed;
        private final ReentrantReadWriteLock lock;
        
        public Account(BigDecimal initialBalance) {
            this.balance = initialBalance;
            this.closed = false;
            this.lock = new ReentrantReadWriteLock();
        }
        
        public boolean deposit(BigDecimal amount) {
            lock.writeLock().lock();
            try {
                if (closed) {
                    return false;
                }
                if (amount.compareTo(BigDecimal.ZERO) <= 0) {
                    return false;
                }
                balance = balance.add(amount);
                return true;
            } finally {
                lock.writeLock().unlock();
            }
        }
        
        public boolean withdraw(BigDecimal amount) {
            lock.writeLock().lock();
            try {
                if (closed) {
                    return false;
                }
                if (amount.compareTo(BigDecimal.ZERO) <= 0) {
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
    }
    
    private final ConcurrentHashMap<String, Account> accounts;
    
    public Task181() {
        this.accounts = new ConcurrentHashMap<>();
    }
    
    public boolean openAccount(String accountId, BigDecimal initialBalance) {
        if (accountId == null || initialBalance == null || initialBalance.compareTo(BigDecimal.ZERO) < 0) {
            return false;
        }
        Account newAccount = new Account(initialBalance);
        return accounts.putIfAbsent(accountId, newAccount) == null;
    }
    
    public boolean closeAccount(String accountId) {
        Account account = accounts.get(accountId);
        if (account == null) {
            return false;
        }
        return account.close();
    }
    
    public boolean deposit(String accountId, BigDecimal amount) {
        Account account = accounts.get(accountId);
        if (account == null) {
            return false;
        }
        return account.deposit(amount);
    }
    
    public boolean withdraw(String accountId, BigDecimal amount) {
        Account account = accounts.get(accountId);
        if (account == null) {
            return false;
        }
        return account.withdraw(amount);
    }
    
    public BigDecimal getBalance(String accountId) {
        Account account = accounts.get(accountId);
        if (account == null) {
            return null;
        }
        return account.getBalance();
    }
    
    public static void main(String[] args) {
        Task181 bank = new Task181();
        
        // Test Case 1: Open account and check balance
        System.out.println("Test 1: Open account");
        boolean opened = bank.openAccount("ACC001", new BigDecimal("1000.00"));
        System.out.println("Account opened: " + opened);
        System.out.println("Balance: " + bank.getBalance("ACC001"));
        
        // Test Case 2: Deposit money
        System.out.println("\\nTest 2: Deposit money");
        boolean deposited = bank.deposit("ACC001", new BigDecimal("500.00"));
        System.out.println("Deposit successful: " + deposited);
        System.out.println("Balance: " + bank.getBalance("ACC001"));
        
        // Test Case 3: Withdraw money
        System.out.println("\\nTest 3: Withdraw money");
        boolean withdrawn = bank.withdraw("ACC001", new BigDecimal("300.00"));
        System.out.println("Withdrawal successful: " + withdrawn);
        System.out.println("Balance: " + bank.getBalance("ACC001"));
        
        // Test Case 4: Close account and try operations
        System.out.println("\\nTest 4: Close account");
        boolean closed = bank.closeAccount("ACC001");
        System.out.println("Account closed: " + closed);
        boolean depositAfterClose = bank.deposit("ACC001", new BigDecimal("100.00"));
        System.out.println("Deposit after close: " + depositAfterClose);
        BigDecimal balanceAfterClose = bank.getBalance("ACC001");
        System.out.println("Balance after close: " + balanceAfterClose);
        
        // Test Case 5: Insufficient funds
        System.out.println("\\nTest 5: Insufficient funds");
        bank.openAccount("ACC002", new BigDecimal("100.00"));
        boolean withdrawMore = bank.withdraw("ACC002", new BigDecimal("200.00"));
        System.out.println("Withdrawal (insufficient funds): " + withdrawMore);
        System.out.println("Balance: " + bank.getBalance("ACC002"));
    }
}
