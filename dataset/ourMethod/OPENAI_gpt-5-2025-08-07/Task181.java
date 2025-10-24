import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.locks.ReentrantLock;

public class Task181 {

    // Error codes (balances are >= 0 so negative values indicate errors)
    public static final long ERR_NO_ACCOUNT_OR_CLOSED = -1L;
    public static final long ERR_INSUFFICIENT_FUNDS = -2L;
    public static final long ERR_INVALID_INPUT = -3L;

    static final class Account {
        private final ReentrantLock lock = new ReentrantLock();
        private long balance; // in cents
        private boolean closed;

        Account(long initial) {
            this.balance = initial;
            this.closed = false;
        }

        ReentrantLock lock() {
            return lock;
        }

        boolean isClosed() {
            return closed;
        }

        long getBalance() {
            return balance;
        }

        void setBalance(long newBalance) {
            balance = newBalance;
        }

        void close() {
            closed = true;
        }
    }

    static final class Bank {
        private final ConcurrentHashMap<String, Account> accounts = new ConcurrentHashMap<>();

        private boolean validId(String id) {
            if (id == null) return false;
            byte[] bytes = id.getBytes(StandardCharsets.UTF_8);
            if (bytes.length == 0 || bytes.length > 32) return false;
            for (int i = 0; i < id.length(); i++) {
                char c = id.charAt(i);
                if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-')) {
                    return false;
                }
            }
            return true;
        }

        public boolean openAccount(String id, long initialCents) {
            if (!validId(id) || initialCents < 0) return false;
            try {
                return accounts.compute(id, (k, v) -> {
                    if (v != null) {
                        return v; // already exists, do not replace
                    }
                    return new Account(initialCents);
                }) != null && accounts.get(id).getBalance() == initialCents && !accounts.get(id).isClosed();
            } catch (Exception e) {
                return false;
            }
        }

        public long getBalance(String id) {
            if (!validId(id)) return ERR_INVALID_INPUT;
            Account acc = accounts.get(id);
            if (acc == null) return ERR_NO_ACCOUNT_OR_CLOSED;
            ReentrantLock l = acc.lock();
            l.lock();
            try {
                if (acc.isClosed()) return ERR_NO_ACCOUNT_OR_CLOSED;
                return acc.getBalance();
            } finally {
                l.unlock();
            }
        }

        public long deposit(String id, long amountCents) {
            if (!validId(id) || amountCents < 0) return ERR_INVALID_INPUT;
            Account acc = accounts.get(id);
            if (acc == null) return ERR_NO_ACCOUNT_OR_CLOSED;
            ReentrantLock l = acc.lock();
            l.lock();
            try {
                if (acc.isClosed()) return ERR_NO_ACCOUNT_OR_CLOSED;
                long newBal = acc.getBalance() + amountCents;
                if (newBal < 0) return ERR_INVALID_INPUT; // overflow guard (unlikely with cents)
                acc.setBalance(newBal);
                return newBal;
            } finally {
                l.unlock();
            }
        }

        public long withdraw(String id, long amountCents) {
            if (!validId(id) || amountCents < 0) return ERR_INVALID_INPUT;
            Account acc = accounts.get(id);
            if (acc == null) return ERR_NO_ACCOUNT_OR_CLOSED;
            ReentrantLock l = acc.lock();
            l.lock();
            try {
                if (acc.isClosed()) return ERR_NO_ACCOUNT_OR_CLOSED;
                long bal = acc.getBalance();
                if (bal < amountCents) return ERR_INSUFFICIENT_FUNDS;
                acc.setBalance(bal - amountCents);
                return acc.getBalance();
            } finally {
                l.unlock();
            }
        }

        public boolean closeAccount(String id) {
            if (!validId(id)) return false;
            Account acc = accounts.get(id);
            if (acc == null) return false;
            ReentrantLock l = acc.lock();
            l.lock();
            try {
                if (acc.isClosed()) return false;
                if (acc.getBalance() != 0) return false; // require zero balance to close
                acc.close();
                return true;
            } finally {
                l.unlock();
            }
        }
    }

    // Test runner
    public static void main(String[] args) {
        Bank bank = new Bank();

        // Test 1: Open and balance check
        boolean opened1 = bank.openAccount("alice", 10_000);
        long bal1 = bank.getBalance("alice");
        System.out.println("Test1: opened=" + opened1 + " balance=" + bal1);

        // Test 2: Deposit
        long bal2 = bank.deposit("alice", 2_500);
        System.out.println("Test2: deposit newBalance=" + bal2);

        // Test 3: Withdraw success
        long bal3 = bank.withdraw("alice", 5_000);
        System.out.println("Test3: withdraw newBalance=" + bal3);

        // Test 4: Withdraw insufficient
        long w4 = bank.withdraw("alice", 10_000);
        long bal4 = bank.getBalance("alice");
        System.out.println("Test4: withdrawResult=" + w4 + " balanceAfter=" + bal4);

        // Test 5: Concurrency + close behavior
        String accId = "bob";
        boolean opened2 = bank.openAccount(accId, 0);
        int depositThreads = 4;
        int depositIters = 1000;
        long depositAmt = 100;
        int withdrawThreads = 2;
        int withdrawIters = 500;
        long withdrawAmt = 50;
        CountDownLatch start = new CountDownLatch(1);
        CountDownLatch done = new CountDownLatch(depositThreads + withdrawThreads);
        List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < depositThreads; i++) {
            Thread t = new Thread(() -> {
                try {
                    start.await();
                    for (int j = 0; j < depositIters; j++) {
                        bank.deposit(accId, depositAmt);
                    }
                } catch (InterruptedException ignored) {
                    Thread.currentThread().interrupt();
                } finally {
                    done.countDown();
                }
            });
            threads.add(t);
            t.start();
        }
        for (int i = 0; i < withdrawThreads; i++) {
            Thread t = new Thread(() -> {
                try {
                    start.await();
                    for (int j = 0; j < withdrawIters; j++) {
                        long r = bank.withdraw(accId, withdrawAmt);
                        if (r == ERR_INSUFFICIENT_FUNDS) {
                            // skip if not enough balance yet
                        }
                    }
                } catch (InterruptedException ignored) {
                    Thread.currentThread().interrupt();
                } finally {
                    done.countDown();
                }
            });
            threads.add(t);
            t.start();
        }
        start.countDown();
        try {
            done.await();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        long finalExpected = (long) depositThreads * depositIters * depositAmt - (long) withdrawThreads * withdrawIters * withdrawAmt;
        long bal5 = bank.getBalance(accId);
        boolean closeAttemptNonZero = bank.closeAccount(accId); // expect false if non-zero
        long withdrawAll = bal5 >= 0 ? bank.withdraw(accId, Math.max(0, bal5)) : bal5;
        boolean closeAttemptZero = bank.closeAccount(accId); // expect true now
        long postCloseDeposit = bank.deposit(accId, 100);
        System.out.println("Test5: opened=" + opened2
                + " expectedBalance=" + finalExpected
                + " actualBalance=" + bal5
                + " closeNonZero=" + closeAttemptNonZero
                + " withdrawAllRes=" + withdrawAll
                + " closeZero=" + closeAttemptZero
                + " depositAfterCloseRes=" + postCloseDeposit);
    }
}