import java.util.concurrent.Semaphore;
import java.util.function.IntConsumer;

public class Task196 {

    // ZeroEvenOdd implementation using semaphores
    static final class ZeroEvenOdd {
        private final int n;
        private final Semaphore zeroSem = new Semaphore(1);
        private final Semaphore evenSem = new Semaphore(0);
        private final Semaphore oddSem = new Semaphore(0);

        public ZeroEvenOdd(int n) {
            if (n < 1 || n > 1000) {
                throw new IllegalArgumentException("n must be in [1, 1000]");
            }
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

    public static void main(String[] args) {
        int[] tests = new int[] {1, 2, 3, 5, 10};
        for (int n : tests) {
            ZeroEvenOdd zeo = new ZeroEvenOdd(n);
            StringBuilder sb = new StringBuilder();

            IntConsumer printNumber = (int x) -> {
                synchronized (sb) { sb.append(x); }
                System.out.print(x);
            };

            Thread tZero = new Thread(() -> {
                try { zeo.zero(printNumber); } catch (InterruptedException e) { Thread.currentThread().interrupt(); }
            });
            Thread tEven = new Thread(() -> {
                try { zeo.even(printNumber); } catch (InterruptedException e) { Thread.currentThread().interrupt(); }
            });
            Thread tOdd = new Thread(() -> {
                try { zeo.odd(printNumber); } catch (InterruptedException e) { Thread.currentThread().interrupt(); }
            });

            tZero.start();
            tEven.start();
            tOdd.start();

            try {
                tZero.join();
                tEven.join();
                tOdd.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }

            System.out.println(" -> " + sb.toString());
        }
    }
}