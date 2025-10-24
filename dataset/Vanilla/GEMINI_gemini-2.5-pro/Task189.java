import java.util.concurrent.Semaphore;
import java.util.function.IntConsumer;

class ZeroEvenOdd {
    private int n;
    private Semaphore semZero;
    private Semaphore semEven;
    private Semaphore semOdd;

    public ZeroEvenOdd(int n) {
        this.n = n;
        this.semZero = new Semaphore(1);
        this.semEven = new Semaphore(0);
        this.semOdd = new Semaphore(0);
    }

    // printNumber.accept(x) outputs "x", where x is an integer.
    public void zero(IntConsumer printNumber) throws InterruptedException {
        for (int i = 1; i <= n; i++) {
            semZero.acquire();
            printNumber.accept(0);
            if (i % 2 == 1) { // Next is odd
                semOdd.release();
            } else { // Next is even
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

public class Task189 {
    public static void main(String[] args) {
        int[] testCases = {2, 5, 1, 6, 7};
        for (int n : testCases) {
            System.out.println("Running test for n = " + n);
            ZeroEvenOdd zeroEvenOdd = new ZeroEvenOdd(n);
            
            IntConsumer printNumber = x -> System.out.print(x);

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
                e.printStackTrace();
            }
            System.out.println("\n");
        }
    }
}