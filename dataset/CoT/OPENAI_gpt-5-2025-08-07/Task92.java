import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

public class Task92 {
    private static class SharedCounter {
        private int currentCounter;
        private final int maxCounter;
        private final ReentrantLock lock = new ReentrantLock();

        SharedCounter(int start, int max) {
            this.currentCounter = start;
            this.maxCounter = max;
        }

        public boolean incrementIfAllowed(String threadName) {
            lock.lock();
            try {
                if (currentCounter <= maxCounter) {
                    currentCounter++;
                    System.out.println("Thread " + threadName + " incremented currentCounter to " + currentCounter);
                    return true;
                }
                return false;
            } finally {
                lock.unlock();
            }
        }

        public int getCurrent() {
            return currentCounter;
        }
    }

    public static int runCounter(int maxCounter, int threadCount) {
        if (threadCount <= 0) {
            return 0;
        }
        SharedCounter shared = new SharedCounter(0, maxCounter);
        List<Thread> threads = new ArrayList<>(threadCount);
        for (int i = 0; i < threadCount; i++) {
            final String name = "T-" + i;
            Thread t = new Thread(() -> {
                while (true) {
                    if (!shared.incrementIfAllowed(Thread.currentThread().getName())) {
                        break;
                    }
                    Thread.yield();
                }
            }, name);
            threads.add(t);
            t.start();
        }
        for (Thread t : threads) {
            try {
                t.join();
            } catch (InterruptedException ie) {
                Thread.currentThread().interrupt();
                // In a real application, handle interruption appropriately.
            }
        }
        return shared.getCurrent();
    }

    public static void main(String[] args) {
        System.out.println("Final count (max=5, threads=3): " + runCounter(5, 3));
        System.out.println("Final count (max=8, threads=4): " + runCounter(8, 4));
        System.out.println("Final count (max=0, threads=2): " + runCounter(0, 2));
        System.out.println("Final count (max=1, threads=1): " + runCounter(1, 1));
        System.out.println("Final count (max=3, threads=5): " + runCounter(3, 5));
    }
}