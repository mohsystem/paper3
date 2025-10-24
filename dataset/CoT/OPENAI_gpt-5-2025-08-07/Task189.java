import java.util.concurrent.Semaphore;

public class Task189 {

    @FunctionalInterface
    public interface IntPrinter {
        void printNumber(int x);
    }

    public static class ZeroEvenOdd {
        private final int n;
        private final Semaphore zeroSem = new Semaphore(1);
        private final Semaphore evenSem = new Semaphore(0);
        private final Semaphore oddSem = new Semaphore(0);

        public ZeroEvenOdd(int n) {
            if (n < 1 || n > 1000) {
                throw new IllegalArgumentException("n out of bounds");
            }
            this.n = n;
        }

        public void zero(IntPrinter printNumber) throws InterruptedException {
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

        public void even(IntPrinter printNumber) throws InterruptedException {
            for (int i = 2; i <= n; i += 2) {
                evenSem.acquire();
                printNumber.printNumber(i);
                zeroSem.release();
            }
        }

        public void odd(IntPrinter printNumber) throws InterruptedException {
            for (int i = 1; i <= n; i += 2) {
                oddSem.acquire();
                printNumber.printNumber(i);
                zeroSem.release();
            }
        }
    }

    // Runs one test and returns the produced sequence as String
    public static String runOnce(int n) {
        StringBuilder sb = new StringBuilder();
        Object lock = new Object();
        IntPrinter printer = (x) -> {
            synchronized (lock) {
                sb.append(x);
            }
        };

        ZeroEvenOdd zeo = new ZeroEvenOdd(n);

        Thread tZero = new Thread(() -> {
            try {
                zeo.zero(printer);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        Thread tEven = new Thread(() -> {
            try {
                zeo.even(printer);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        Thread tOdd = new Thread(() -> {
            try {
                zeo.odd(printer);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
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

        return sb.toString();
    }

    public static void main(String[] args) {
        int[] tests = {1, 2, 5, 7, 10};
        for (int n : tests) {
            String out = runOnce(n);
            System.out.println("n=" + n + ": " + out);
        }
    }
}