import java.util.concurrent.Semaphore;

public class Task189 {
    interface Printer {
        void printNumber(int x);
    }

    static final class ZeroEvenOdd {
        private final int n;
        private final Semaphore zeroSem = new Semaphore(1);
        private final Semaphore evenSem = new Semaphore(0);
        private final Semaphore oddSem = new Semaphore(0);

        public ZeroEvenOdd(int n) {
            if (n < 1 || n > 1000) {
                throw new IllegalArgumentException("n out of range");
            }
            this.n = n;
        }

        public void zero(Printer printNumber) throws InterruptedException {
            for (int i = 1; i <= n; i++) {
                zeroSem.acquire();
                printNumber.printNumber(0);
                if ((i & 1) == 1) {
                    oddSem.release();
                } else {
                    evenSem.release();
                }
            }
        }

        public void even(Printer printNumber) throws InterruptedException {
            for (int i = 2; i <= n; i += 2) {
                evenSem.acquire();
                printNumber.printNumber(i);
                zeroSem.release();
            }
        }

        public void odd(Printer printNumber) throws InterruptedException {
            for (int i = 1; i <= n; i += 2) {
                oddSem.acquire();
                printNumber.printNumber(i);
                zeroSem.release();
            }
        }
    }

    public static void main(String[] args) {
        int[] tests = {1, 2, 5, 7, 10};
        for (int n : tests) {
            ZeroEvenOdd zeo = new ZeroEvenOdd(n);
            Printer printer = new Printer() {
                @Override
                public void printNumber(int x) {
                    System.out.print(x);
                }
            };

            Thread tZero = new Thread(() -> {
                try { zeo.zero(printer); } catch (InterruptedException ignored) { Thread.currentThread().interrupt(); }
            });
            Thread tEven = new Thread(() -> {
                try { zeo.even(printer); } catch (InterruptedException ignored) { Thread.currentThread().interrupt(); }
            });
            Thread tOdd = new Thread(() -> {
                try { zeo.odd(printer); } catch (InterruptedException ignored) { Thread.currentThread().interrupt(); }
            });

            tZero.start();
            tEven.start();
            tOdd.start();

            try {
                tZero.join();
                tEven.join();
                tOdd.join();
            } catch (InterruptedException ignored) {
                Thread.currentThread().interrupt();
            }
            System.out.println();
        }
    }
}