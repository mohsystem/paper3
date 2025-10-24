import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class Task92 {

    static class SharedResource {
        private int currentCounter = 0;
        private final int maxCounter;
        private final Lock lock = new ReentrantLock();

        public SharedResource(int maxCounter) {
            this.maxCounter = maxCounter;
        }

        public void accessResource() {
            // This loop allows threads to compete for access until the counter is maxed out.
            while (true) {
                int localValue = -1;
                boolean shouldBreak = false;

                lock.lock();
                try {
                    // Check-then-act is performed atomically inside the lock to prevent TOCTOU.
                    if (currentCounter < maxCounter) {
                        currentCounter++;
                        localValue = currentCounter;
                    } else {
                        shouldBreak = true;
                    }
                } finally {
                    lock.unlock();
                }

                if (shouldBreak) {
                    break;
                }

                if (localValue != -1) {
                    System.out.println("Thread " + Thread.currentThread().getName() + " is accessing counter: " + localValue);
                }
            }
        }
    }

    static class Worker implements Runnable {
        private final SharedResource resource;

        public Worker(SharedResource resource) {
            this.resource = resource;
        }

        @Override
        public void run() {
            resource.accessResource();
        }
    }

    public static void runTest(int numThreads, int maxCounter) {
        System.out.println("--- Running test with " + numThreads + " threads and max counter " + maxCounter + " ---");
        SharedResource resource = new SharedResource(maxCounter);
        List<Thread> threads = new ArrayList<>();

        for (int i = 0; i < numThreads; i++) {
            Thread t = new Thread(new Worker(resource), "Worker-" + (i + 1));
            threads.add(t);
            t.start();
        }

        for (Thread t : threads) {
            try {
                t.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                System.err.println("Thread interrupted: " + e.getMessage());
            }
        }
        System.out.println("--- Test finished. Final counter should be " + maxCounter + " ---");
        System.out.println();
    }

    public static void main(String[] args) {
        // 5 test cases
        runTest(5, 10);
        runTest(10, 20);
        runTest(2, 50);
        runTest(20, 100);
        runTest(8, 8);
    }
}