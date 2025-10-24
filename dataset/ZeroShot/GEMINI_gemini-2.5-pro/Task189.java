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

        // printNumber.accept(x) outputs "x", where x is an integer.
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
            System.out.println("Test Case n = " + n);
            System.out.print("Output: ");
            ZeroEvenOdd zeo = new ZeroEvenOdd(n);
            
            IntConsumer printNumber = (x) -> System.out.print(x);

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
            System.out.println("\n");
        }
    }
}