import java.util.concurrent.Semaphore;
import java.util.function.IntConsumer;

public class Task189 {

    // Prints number to console
    public static void printNumber(int x) {
        System.out.print(x);
    }

    static class ZeroEvenOdd {
        private int n;
        private final Semaphore zeroSem = new Semaphore(1);
        private final Semaphore evenSem = new Semaphore(0);
        private final Semaphore oddSem = new Semaphore(0);

        public ZeroEvenOdd(int n) {
            this.n = n;
        }

        public void zero(IntConsumer printNumber) {
            try {
                for (int i = 1; i <= n; i++) {
                    zeroSem.acquire();
                    printNumber.accept(0);
                    if ((i & 1) == 1) {
                        oddSem.release();
                    } else {
                        evenSem.release();
                    }
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }

        public void even(IntConsumer printNumber) {
            try {
                for (int i = 2; i <= n; i += 2) {
                    evenSem.acquire();
                    printNumber.accept(i);
                    zeroSem.release();
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }

        public void odd(IntConsumer printNumber) {
            try {
                for (int i = 1; i <= n; i += 2) {
                    oddSem.acquire();
                    printNumber.accept(i);
                    zeroSem.release();
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }

    // Runs the ZeroEvenOdd with threads; returns the produced string (also prints to console)
    public static String runZeroEvenOdd(int n) {
        ZeroEvenOdd zeo = new ZeroEvenOdd(n);
        StringBuilder sb = new StringBuilder();
        IntConsumer p = x -> {
            synchronized (sb) { sb.append(x); }
            printNumber(x);
        };

        Thread tA = new Thread(() -> zeo.zero(p));
        Thread tB = new Thread(() -> zeo.even(p));
        Thread tC = new Thread(() -> zeo.odd(p));
        tA.start();
        tB.start();
        tC.start();
        try {
            tA.join();
            tB.join();
            tC.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        int[] tests = {1, 2, 5, 6, 7};
        for (int n : tests) {
            runZeroEvenOdd(n);
            System.out.println();
        }
    }
}