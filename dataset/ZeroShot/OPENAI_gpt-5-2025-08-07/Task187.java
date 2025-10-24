import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.Semaphore;

public class Task187 {
    // H2O synchronization class
    static final class H2O {
        private final Semaphore hSem = new Semaphore(2, true);
        private final Semaphore oSem = new Semaphore(1, true);
        private final CyclicBarrier barrier;

        H2O() {
            // Barrier action replenishes permits for the next molecule
            this.barrier = new CyclicBarrier(3, () -> {
                hSem.release(2);
                oSem.release(1);
            });
        }

        public void hydrogen(Runnable releaseHydrogen) {
            boolean acquired = false;
            try {
                hSem.acquire();
                acquired = true;
                releaseHydrogen.run();
                barrierAwaitQuietly();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            } finally {
                // If thread was interrupted before reaching barrier, avoid permit leak
                if (!acquired) {
                    // no-op, acquire would have thrown before consuming a permit
                }
            }
        }

        public void oxygen(Runnable releaseOxygen) {
            boolean acquired = false;
            try {
                oSem.acquire();
                acquired = true;
                releaseOxygen.run();
                barrierAwaitQuietly();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            } finally {
                if (!acquired) {
                    // no-op
                }
            }
        }

        private void barrierAwaitQuietly() {
            try {
                barrier.await();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            } catch (BrokenBarrierException e) {
                // Barrier broken due to unexpected interruption; ignore for this controlled setup
            }
        }
    }

    // Builds water output from input arrival string
    public static String buildWater(String water) {
        if (water == null) return "";
        int n = water.length();
        final StringBuilder sb = new StringBuilder(n);
        final Object lock = new Object();
        H2O h2o = new H2O();

        Runnable releaseH = () -> {
            synchronized (lock) {
                sb.append('H');
            }
        };
        Runnable releaseO = () -> {
            synchronized (lock) {
                sb.append('O');
            }
        };

        List<Thread> threads = new ArrayList<>(n);
        for (int i = 0; i < n; i++) {
            char c = water.charAt(i);
            Thread t;
            if (c == 'H') {
                t = new Thread(() -> h2o.hydrogen(releaseH), "H-" + i);
            } else if (c == 'O') {
                t = new Thread(() -> h2o.oxygen(releaseO), "O-" + i);
            } else {
                continue; // skip invalid chars
            }
            threads.add(t);
        }

        for (Thread t : threads) t.start();

        // Join with a timeout to avoid indefinite hang in case of unexpected issues
        for (Thread t : threads) {
            try {
                t.join(10000);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
            if (t.isAlive()) {
                t.interrupt();
            }
        }

        return sb.toString();
    }

    // Simple verifier: counts in groups of 3 contain 2 H and 1 O
    private static boolean isValidWater(String s) {
        if (s == null || s.length() % 3 != 0) return false;
        for (int i = 0; i < s.length(); i += 3) {
            int h = 0, o = 0;
            for (int j = 0; j < 3; j++) {
                char c = s.charAt(i + j);
                if (c == 'H') h++;
                else if (c == 'O') o++;
                else return false;
            }
            if (h != 2 || o != 1) return false;
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "HOH",
            "OOHHHH",
            "HHOOHH",
            "OHOHHH",
            "HHOHHO"
        };
        for (String in : tests) {
            String out = buildWater(in);
            System.out.println("in=" + in + " out=" + out + " valid=" + isValidWater(out));
        }
    }
}