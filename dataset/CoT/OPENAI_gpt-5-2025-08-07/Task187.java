import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

public class Task187 {

    // Synchronization class for H2O formation
    static class H2O {
        private final Semaphore hSem = new Semaphore(2, true);
        private final Semaphore oSem = new Semaphore(1, true);
        private final CyclicBarrier barrier = new CyclicBarrier(3);

        public void hydrogen(Runnable releaseHydrogen) {
            try {
                hSem.acquire();
                try {
                    releaseHydrogen.run();
                    barrier.await();
                } finally {
                    hSem.release();
                }
            } catch (InterruptedException ie) {
                Thread.currentThread().interrupt();
            } catch (BrokenBarrierException bbe) {
                // Barrier broken, allow graceful exit
            }
        }

        public void oxygen(Runnable releaseOxygen) {
            try {
                oSem.acquire();
                try {
                    releaseOxygen.run();
                    barrier.await();
                } finally {
                    oSem.release();
                }
            } catch (InterruptedException ie) {
                Thread.currentThread().interrupt();
            } catch (BrokenBarrierException bbe) {
                // Barrier broken, allow graceful exit
            }
        }
    }

    // Runs the water formation concurrently and returns the resulting sequence
    public static String runWater(String water) {
        if (water == null) return "";
        final StringBuffer out = new StringBuffer(water.length());
        H2O h2o = new H2O();
        List<Thread> threads = new ArrayList<>(water.length());

        Runnable releaseHydrogen = () -> out.append('H');
        Runnable releaseOxygen = () -> out.append('O');

        for (int i = 0; i < water.length(); i++) {
            char c = water.charAt(i);
            if (c == 'H') {
                Thread t = new Thread(() -> h2o.hydrogen(releaseHydrogen));
                threads.add(t);
                t.start();
            } else if (c == 'O') {
                Thread t = new Thread(() -> h2o.oxygen(releaseOxygen));
                threads.add(t);
                t.start();
            } else {
                // ignore invalid characters
            }
        }

        for (Thread t : threads) {
            try {
                t.join();
            } catch (InterruptedException ie) {
                Thread.currentThread().interrupt();
            }
        }
        return out.toString();
    }

    // Validate output: each group of 3 must contain exactly 2 H and 1 O
    public static boolean validateWater(String out) {
        if (out == null || out.length() % 3 != 0) return false;
        for (int i = 0; i < out.length(); i += 3) {
            int h = 0, o = 0;
            for (int j = i; j < i + 3; j++) {
                char c = out.charAt(j);
                if (c == 'H') h++;
                else if (c == 'O') o++;
                else return false;
            }
            if (!(h == 2 && o == 1)) return false;
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "HOH",
            "OOHHHH",
            "HHOHHO",
            "OHHHOH",
            "HHOOHH"
        };
        for (String in : tests) {
            String out = runWater(in);
            System.out.println("Input:  " + in);
            System.out.println("Output: " + out + " | valid=" + validateWater(out));
            System.out.println("---");
        }
    }
}