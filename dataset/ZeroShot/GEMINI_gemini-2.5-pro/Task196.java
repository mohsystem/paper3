import java.util.concurrent.Semaphore;
import java.util.function.IntConsumer;

public class Task196 {

    static class ZeroEvenOdd {
        private int n;
        private Semaphore semZero;
        private Semaphore semEven;
        private Semaphore semOdd;

        public ZeroEvenOdd(int n) {
            this.n = n;
            this.semZero = new Semaphore(1); // Start with zero
            this.semEven = new Semaphore(0);
            this.semOdd = new Semaphore(0);
        }

        // printNumber.accept(x) outputs "x", where x is an integer.
        public void zero(IntConsumer printNumber) throws InterruptedException {
            for (int i = 1; i <= n; i++) {
                semZero.acquire();
                printNumber.accept(0);
                if (i % 2 == 1) { // Next number is odd
                    semOdd.release();
                } else { // Next number is even
                    semEven.release();
                }
            }
        }

        public void even(IntConsumer printNumber) throws InterruptedException {
            for (int i = 2; i <= n; i += 2) {
                semEven.acquire();
                printNumber.accept(i);
                semZero.release();
            }
        }

        public void odd(IntConsumer printNumber) throws InterruptedException {
            for (int i = 1; i <= n; i += 2) {
                semOdd.acquire();
                printNumber.accept(i);
                semZero.release();
            }
        }
    }

    public static void main(String[] args) {
        int[] testCases = {2, 5, 1, 6, 10};
        for (int n : testCases) {
            System.out.println("Test Case n = " + n + ":");
            runTest(n);
            System.out.println("\n");
        }
    }

    private static void runTest(int n) {
        ZeroEvenOdd zeo = new ZeroEvenOdd(n);
        IntConsumer printNumber = System.out::print;

        Thread threadA = new Thread(() -> {
            try {
                zeo.zero(printNumber);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        Thread threadB = new Thread(() -> {
            try {
                zeo.even(printNumber);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        Thread threadC = new Thread(() -> {
            try {
                zeo.odd(printNumber);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        threadA.start();
        threadB.start();
        threadC.start();

        try {
            threadA.join();
            threadB.join();
            threadC.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }
}