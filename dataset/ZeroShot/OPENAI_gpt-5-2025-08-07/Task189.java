import java.util.concurrent.Semaphore;
import java.util.function.IntConsumer;

public class Task189 {
    // ZeroEvenOdd implementation using semaphores
    static class ZeroEvenOdd {
        private final int n;
        private final Semaphore semZero = new Semaphore(1);
        private final Semaphore semOdd = new Semaphore(0);
        private final Semaphore semEven = new Semaphore(0);

        ZeroEvenOdd(int n) {
            this.n = n;
        }

        public void zero(IntConsumer printNumber) {
            for (int i = 1; i <= n; i++) {
                try {
                    semZero.acquire();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    return;
                }
                try {
                    printNumber.accept(0);
                } catch (RuntimeException ex) {
                    // In case the printNumber throws; ensure not to deadlock
                    semOdd.release();
                    semEven.release();
                    return;
                }
                if ((i & 1) == 1) {
                    semOdd.release();
                } else {
                    semEven.release();
                }
            }
        }

        public void even(IntConsumer printNumber) {
            for (int i = 2; i <= n; i += 2) {
                try {
                    semEven.acquire();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    return;
                }
                printNumber.accept(i);
                semZero.release();
            }
        }

        public void odd(IntConsumer printNumber) {
            for (int i = 1; i <= n; i += 2) {
                try {
                    semOdd.acquire();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    return;
                }
                printNumber.accept(i);
                semZero.release();
            }
        }
    }

    // Runs the ZeroEvenOdd printing, returns the produced string
    public static String runZeroEvenOdd(int n) {
        if (n < 1 || n > 1000) return "";
        ZeroEvenOdd zeo = new ZeroEvenOdd(n);
        final StringBuilder sb = new StringBuilder(n * 5); // capacity sufficient for n up to 1000
        IntConsumer printer = (x) -> {
            synchronized (sb) {
                sb.append(x);
            }
        };
        Thread tZero = new Thread(() -> zeo.zero(printer), "Zero");
        Thread tOdd = new Thread(() -> zeo.odd(printer), "Odd");
        Thread tEven = new Thread(() -> zeo.even(printer), "Even");
        tZero.start();
        tOdd.start();
        tEven.start();
        try {
            tZero.join();
            tOdd.join();
            tEven.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        int[] tests = {1, 2, 5, 7, 10};
        for (int n : tests) {
            System.out.println(runZeroEvenOdd(n));
        }
    }
}