import java.util.concurrent.Semaphore;
import java.util.function.IntConsumer;

public class Task196 {

    static class ZeroEvenOdd {
        private final int n;
        private final Semaphore semZero = new Semaphore(1);
        private final Semaphore semEven = new Semaphore(0);
        private final Semaphore semOdd = new Semaphore(0);

        public ZeroEvenOdd(int n) {
            this.n = n;
        }

        public void zero(IntConsumer printNumber) throws InterruptedException {
            for (int i = 1; i <= n; i++) {
                semZero.acquire();
                printNumber.accept(0);
                if (i % 2 == 0) {
                    semEven.release();
                } else {
                    semOdd.release();
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
            System.out.println("Testing with n = " + n);
            runTest(n);
            System.out.println("\n");
        }
    }

    private static void runTest(int n) {
        ZeroEvenOdd zeroEvenOdd = new ZeroEvenOdd(n);
        IntConsumer printNumber = System.out::print;

        Thread threadA = new Thread(() -> {
            try {
                zeroEvenOdd.zero(printNumber);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        Thread threadB = new Thread(() -> {
            try {
                zeroEvenOdd.even(printNumber);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        Thread threadC = new Thread(() -> {
            try {
                zeroEvenOdd.odd(printNumber);
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