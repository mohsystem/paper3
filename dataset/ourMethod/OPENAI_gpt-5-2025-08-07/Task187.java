import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.Semaphore;

public class Task187 {

    // Synchronization class ensuring 2 H and 1 O form water per barrier trip
    static class H2O {
        private final Semaphore hSem = new Semaphore(2);
        private final Semaphore oSem = new Semaphore(1);
        private final CyclicBarrier barrier;

        public H2O() {
            this.barrier = new CyclicBarrier(3, () -> {
                // Reset permits for the next molecule group
                hSem.release(2);
                oSem.release(1);
            });
        }

        public void hydrogen(Runnable releaseHydrogen) throws InterruptedException {
            hSem.acquire();
            try {
                releaseHydrogen.run();
                barrierAwait();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                throw e;
            }
        }

        public void oxygen(Runnable releaseOxygen) throws InterruptedException {
            oSem.acquire();
            try {
                releaseOxygen.run();
                barrierAwait();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                throw e;
            }
        }

        private void barrierAwait() throws InterruptedException {
            try {
                barrier.await();
            } catch (BrokenBarrierException e) {
                throw new InterruptedException("Barrier broken");
            }
        }
    }

    public static String formWater(String water) {
        if (water == null) return "";
        final int n = water.length();
        if (n % 3 != 0) return "";
        int hCount = 0, oCount = 0;
        for (int i = 0; i < n; i++) {
            char c = water.charAt(i);
            if (c == 'H') hCount++;
            else if (c == 'O') oCount++;
            else return "";
        }
        if (hCount != 2 * oCount) return "";

        final StringBuilder sb = new StringBuilder(n);
        final Object lock = new Object();
        H2O h2o = new H2O();

        List<Thread> threads = new ArrayList<>(n);
        for (int i = 0; i < n; i++) {
            char c = water.charAt(i);
            if (c == 'H') {
                Thread t = new Thread(() -> {
                    try {
                        h2o.hydrogen(() -> {
                            synchronized (lock) {
                                sb.append('H');
                            }
                        });
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                });
                threads.add(t);
            } else {
                Thread t = new Thread(() -> {
                    try {
                        h2o.oxygen(() -> {
                            synchronized (lock) {
                                sb.append('O');
                            }
                        });
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                });
                threads.add(t);
            }
        }

        for (Thread t : threads) t.start();
        for (Thread t : threads) {
            try {
                t.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
        return sb.toString();
    }

    private static boolean validateMolecules(String s) {
        if (s == null || s.length() % 3 != 0) return false;
        for (int i = 0; i < s.length(); i += 3) {
            int h = 0, o = 0;
            for (int j = 0; j < 3; j++) {
                char c = s.charAt(i + j);
                if (c == 'H') h++;
                else if (c == 'O') o++;
                else return false;
            }
            if (!(h == 2 && o == 1)) return false;
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = new String[]{
            "HOH",
            "OOHHHH",
            "HHOOHH",
            "OOOHHHHHH",
            "HHOHHOHHO"
        };
        for (String water : tests) {
            String out = formWater(water);
            System.out.println("Input:  " + water);
            System.out.println("Output: " + out);
            System.out.println("Valid:  " + validateMolecules(out));
            System.out.println("---");
        }
    }
}