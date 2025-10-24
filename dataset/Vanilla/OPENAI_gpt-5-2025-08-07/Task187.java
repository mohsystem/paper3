import java.util.*;
import java.util.concurrent.*;

public class Task187 {
    static class H2O {
        private final Semaphore hSlots = new Semaphore(2);
        private final Semaphore oSlots = new Semaphore(1);

        private final Semaphore mutex = new Semaphore(1);
        private final Semaphore turnstile = new Semaphore(0);
        private final Semaphore turnstile2 = new Semaphore(1);
        private int count = 0;

        private boolean barrierPhase1() throws InterruptedException {
            boolean leader = false;
            mutex.acquire();
            count++;
            if (count == 3) {
                turnstile2.acquire();
                turnstile.release(3);
                leader = true;
            }
            mutex.release();
            turnstile.acquire();
            turnstile.release();
            return leader;
        }

        private void barrierPhase2() throws InterruptedException {
            mutex.acquire();
            count--;
            if (count == 0) {
                turnstile.acquire();
                turnstile2.release(3);
            }
            mutex.release();
            turnstile2.acquire();
            turnstile2.release();
        }

        public void hydrogen(Runnable releaseHydrogen) throws InterruptedException {
            hSlots.acquire();
            boolean leader = barrierPhase1();
            try {
                releaseHydrogen.run();
            } finally {
                barrierPhase2();
                if (leader) {
                    hSlots.release(2);
                    oSlots.release(1);
                }
            }
        }

        public void oxygen(Runnable releaseOxygen) throws InterruptedException {
            oSlots.acquire();
            boolean leader = barrierPhase1();
            try {
                releaseOxygen.run();
            } finally {
                barrierPhase2();
                if (leader) {
                    hSlots.release(2);
                    oSlots.release(1);
                }
            }
        }
    }

    public static String makeWater(String water) throws InterruptedException {
        StringBuffer out = new StringBuffer();
        H2O h2o = new H2O();
        List<Thread> threads = new ArrayList<>();

        for (char c : water.toCharArray()) {
            if (c == 'H') {
                Thread t = new Thread(() -> {
                    try {
                        h2o.hydrogen(() -> {
                            synchronized (out) { out.append('H'); }
                        });
                    } catch (InterruptedException ignored) {}
                });
                threads.add(t);
            } else if (c == 'O') {
                Thread t = new Thread(() -> {
                    try {
                        h2o.oxygen(() -> {
                            synchronized (out) { out.append('O'); }
                        });
                    } catch (InterruptedException ignored) {}
                });
                threads.add(t);
            }
        }

        for (Thread t : threads) t.start();
        for (Thread t : threads) t.join();

        return out.toString();
    }

    public static void main(String[] args) throws Exception {
        String[] tests = {
            "HOH",
            "OOHHHH",
            "HOHOHO",
            "HHOHHO",
            "OOOHHHHHH"
        };
        for (String s : tests) {
            String res = makeWater(s);
            System.out.println("Input: " + s + " -> Output: " + res);
        }
    }
}