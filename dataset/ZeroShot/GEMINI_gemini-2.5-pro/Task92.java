import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class Task92 {

    // Inner class to represent the shared resource
    static class SharedResource {
        private int currentCounter = 0;
        private final int maxCounter;
        private final Lock lock = new ReentrantLock();

        public SharedResource(int maxCounter) {
            this.maxCounter = maxCounter;
        }

        public void accessResource() {
            while (true) {
                boolean shouldBreak = false;
                lock.lock();
                try {
                    // Check if the counter is less than the max
                    if (currentCounter < maxCounter) {
                        currentCounter++;
                        System.out.println(Thread.currentThread().getName() + " is accessing the counter: " + currentCounter);
                    } else {
                        // If max is reached, signal to break the loop
                        shouldBreak = true;
                    }
                } finally {
                    // Always release the lock
                    lock.unlock();
                }

                if (shouldBreak) {
                    break;
                }
                 // Optional: A small sleep can make the thread interleaving more visible, but is not required for correctness.
                 /* try {
                     Thread.sleep(10);
                 } catch (InterruptedException e) {
                     Thread.currentThread().interrupt();
                 } */
            }
        }
    }

    // Inner class for the worker thread
    static class WorkerThread extends Thread {
        private final SharedResource sharedResource;

        public WorkerThread(String name, SharedResource resource) {
            super(name);
            this.sharedResource = resource;
        }

        @Override
        public void run() {
            sharedResource.accessResource();
        }
    }

    public static void main(String[] args) {
        final int NUM_THREADS = 5;
        final int MAX_COUNT = 50;

        SharedResource sharedResource = new SharedResource(MAX_COUNT);
        Thread[] threads = new Thread[NUM_THREADS];

        System.out.println("Starting " + NUM_THREADS + " threads to count up to " + MAX_COUNT);

        // Create and start 5 threads (test cases)
        for (int i = 0; i < NUM_THREADS; i++) {
            threads[i] = new WorkerThread("Thread-" + (i + 1), sharedResource);
            threads[i].start();
        }

        // Wait for all threads to complete
        for (int i = 0; i < NUM_THREADS; i++) {
            try {
                threads[i].join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                System.err.println("Thread interrupted: " + e.getMessage());
            }
        }

        System.out.println("All threads have finished execution.");
    }
}