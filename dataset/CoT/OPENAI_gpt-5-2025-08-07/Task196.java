import java.util.concurrent.Semaphore;

public class Task196 {
    static class ZeroEvenOdd {
        private final int n;
        private final Semaphore zeroSem = new Semaphore(1);
        private final Semaphore evenSem = new Semaphore(0);
        private final Semaphore oddSem = new Semaphore(0);
        private final StringBuilder sb = new StringBuilder();
        private final Object sbLock = new Object();

        ZeroEvenOdd(int n) {
            this.n = n;
        }

        private void printNumber(int x) {
            System.out.print(x);
            synchronized (sbLock) {
                sb.append(x);
            }
        }

        public void zero() {
            try {
                for (int i = 1; i <= n; i++) {
                    zeroSem.acquire();
                    printNumber(0);
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

        public void even() {
            try {
                for (int i = 2; i <= n; i += 2) {
                    evenSem.acquire();
                    printNumber(i);
                    zeroSem.release();
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }

        public void odd() {
            try {
                for (int i = 1; i <= n; i += 2) {
                    oddSem.acquire();
                    printNumber(i);
                    zeroSem.release();
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }

        public String run() throws InterruptedException {
            Thread t0 = new Thread(this::zero, "zero");
            Thread t1 = new Thread(this::even, "even");
            Thread t2 = new Thread(this::odd, "odd");
            t0.start();
            t1.start();
            t2.start();
            t0.join();
            t1.join();
            t2.join();
            return sb.toString();
        }
    }

    public static String runZeroEvenOdd(int n) throws InterruptedException {
        ZeroEvenOdd zeo = new ZeroEvenOdd(n);
        return zeo.run();
    }

    public static void main(String[] args) throws Exception {
        int[] tests = {1, 2, 5, 10, 3};
        for (int n : tests) {
            runZeroEvenOdd(n);
            System.out.println();
        }
    }
}