import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

public class Task92 {
    private static class Shared {
        final ReentrantLock lock = new ReentrantLock();
        volatile int currentCounter = 0; // volatile to allow safe read before locking
        final int maxCounter;
        Shared(int maxCounter) {
            this.maxCounter = maxCounter;
        }
    }

    private static class Worker implements Runnable {
        private final Shared shared;
        private final int id;
        Worker(Shared shared, int id) {
            this.shared = shared;
            this.id = id;
        }
        @Override
        public void run() {
            while (true) {
                if (shared.currentCounter <= shared.maxCounter) { // initial check
                    shared.lock.lock();
                    try {
                        if (shared.currentCounter <= shared.maxCounter) { // re-check inside lock
                            shared.currentCounter++;
                            System.out.println("[Java] Thread-" + id + " accessing currentCounter=" + shared.currentCounter);
                        } else {
                            break;
                        }
                    } finally {
                        shared.lock.unlock();
                    }
                } else {
                    break;
                }
                Thread.yield();
            }
        }
    }

    public static int runCounter(int maxCounter, int numThreads) {
        if (numThreads < 0) numThreads = 0;
        Shared shared = new Shared(maxCounter);
        List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < numThreads; i++) {
            Thread t = new Thread(new Worker(shared, i + 1), "Worker-" + (i + 1));
            threads.add(t);
            t.start();
        }
        for (Thread t : threads) {
            try {
                t.join();
            } catch (InterruptedException ie) {
                Thread.currentThread().interrupt();
                // best effort: continue joining others
            }
        }
        return shared.currentCounter;
    }

    public static void main(String[] args) {
        int[][] tests = {
            {5, 3},
            {10, 5},
            {0, 2},
            {1, 4},
            {50, 10}
        };
        for (int i = 0; i < tests.length; i++) {
            int max = tests[i][0];
            int threads = tests[i][1];
            int result = runCounter(max, threads);
            System.out.println("[Java] Test " + (i + 1) + " final currentCounter=" + result + " (max=" + max + ", threads=" + threads + ")");
        }
    }
}