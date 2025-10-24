import java.util.concurrent.Semaphore;
import java.util.function.IntConsumer;

public class Task189 {

    static class ZeroEvenOdd {
        private int n;
        private Semaphore zeroSem = new Semaphore(1);
        private Semaphore evenSem = new Semaphore(0);
        private Semaphore oddSem = new Semaphore(0);

        public ZeroEvenOdd(int n) {
            this.n = n;
        }

        // printNumber.accept(x) outputs x to the console.
        public void zero(IntConsumer printNumber) throws InterruptedException {
            for (int i = 1; i <= n; i++) {
                zeroSem.acquire();
                printNumber.accept(0);
                if (i % 2 == 1) {
                    oddSem.release();
                } else {
                    evenSem.release();
                }
            }
        }

        public void even(IntConsumer printNumber) throws InterruptedException {
            for (int i = 2; i <= n; i += 2) {
                evenSem.acquire();
                printNumber.accept(i);
                zeroSem.release();
            }
        }

        public void odd(IntConsumer printNumber) throws InterruptedException {
            for (int i = 1; i <= n; i += 2) {
                oddSem.acquire();
                printNumber.accept(i);
                zeroSem.release();
            }
        }
    }

    public static void main(String[] args) {
        int[] testCases = {1, 2, 5, 6, 10};

        for (int n : testCases) {
            System.out.print("Test Case n = " + n + ": ");
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
            System.out.println("\n");
        }
    }
}