import java.util.concurrent.Semaphore;
import java.util.function.IntConsumer;

public class Task196 {
    static class ZeroEvenOdd {
        private int n;
        private final Semaphore zeroSem = new Semaphore(1);
        private final Semaphore oddSem = new Semaphore(0);
        private final Semaphore evenSem = new Semaphore(0);

        public ZeroEvenOdd(int n) {
            this.n = n;
        }

        public void zero(IntConsumer printNumber) throws InterruptedException {
            for (int i = 1; i <= n; i++) {
                zeroSem.acquire();
                printNumber.accept(0);
                if ((i & 1) == 1) {
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

    public static void main(String[] args) throws Exception {
        int[] tests = {1, 2, 5, 7, 10};
        for (int n : tests) {
            ZeroEvenOdd zeo = new ZeroEvenOdd(n);
            Thread tZero = new Thread(() -> {
                try {
                    zeo.zero((int x) -> System.out.print(x));
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            });
            Thread tEven = new Thread(() -> {
                try {
                    zeo.even((int x) -> System.out.print(x));
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            });
            Thread tOdd = new Thread(() -> {
                try {
                    zeo.odd((int x) -> System.out.print(x));
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            });

            tZero.start();
            tEven.start();
            tOdd.start();
            tZero.join();
            tEven.join();
            tOdd.join();
            System.out.println();
        }
    }
}