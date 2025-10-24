import java.util.concurrent.Semaphore;

public class Task196 {

    @FunctionalInterface
    interface PrintNumber {
        void print(int x);
    }

    static final class ZeroEvenOdd {
        private final int n;
        private final Semaphore zeroSem = new Semaphore(1);
        private final Semaphore evenSem = new Semaphore(0);
        private final Semaphore oddSem = new Semaphore(0);

        ZeroEvenOdd(int n) {
            if (n < 1 || n > 1000) {
                throw new IllegalArgumentException("n must be in [1, 1000]");
            }
            this.n = n;
        }

        public void zero(PrintNumber printNumber) {
            for (int i = 1; i <= n; i++) {
                try {
                    zeroSem.acquire();
                    printNumber.print(0);
                    if ((i & 1) == 1) {
                        oddSem.release();
                    } else {
                        evenSem.release();
                    }
                } catch (InterruptedException ie) {
                    Thread.currentThread().interrupt();
                    return;
                }
            }
        }

        public void even(PrintNumber printNumber) {
            for (int i = 2; i <= n; i += 2) {
                try {
                    evenSem.acquire();
                    printNumber.print(i);
                    zeroSem.release();
                } catch (InterruptedException ie) {
                    Thread.currentThread().interrupt();
                    return;
                }
            }
        }

        public void odd(PrintNumber printNumber) {
            for (int i = 1; i <= n; i += 2) {
                try {
                    oddSem.acquire();
                    printNumber.print(i);
                    zeroSem.release();
                } catch (InterruptedException ie) {
                    Thread.currentThread().interrupt();
                    return;
                }
            }
        }
    }

    public static String runZeroEvenOdd(int n) {
        ZeroEvenOdd zeo = new ZeroEvenOdd(n);
        StringBuilder sb = new StringBuilder();
        final Object lock = new Object();

        PrintNumber printer = x -> {
            synchronized (lock) {
                System.out.print(x);
                sb.append(x);
            }
        };

        Thread tZero = new Thread(() -> zeo.zero(printer), "zero-thread");
        Thread tEven = new Thread(() -> zeo.even(printer), "even-thread");
        Thread tOdd = new Thread(() -> zeo.odd(printer), "odd-thread");

        tZero.start();
        tEven.start();
        tOdd.start();

        try {
            tZero.join();
            tEven.join();
            tOdd.join();
        } catch (InterruptedException ie) {
            Thread.currentThread().interrupt();
        }
        System.out.println();
        return sb.toString();
    }

    public static void main(String[] args) {
        int[] tests = {1, 2, 5, 10, 3};
        for (int n : tests) {
            String out = runZeroEvenOdd(n);
            System.out.println("Returned: " + out);
        }
    }
}