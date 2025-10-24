import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class Task92 {
    private static int currentCounter = 0;
    private static int maxCounter = 20;
    private static final Lock lock = new ReentrantLock();

    // The function to be executed by each thread
    public static void accessResource() {
        while (true) {
            lock.lock();
            try {
                if (currentCounter < maxCounter) {
                    currentCounter++;
                    System.out.println("Thread " + Thread.currentThread().getId() + " is accessing counter: " + currentCounter);
                } else {
                    // Exit the loop if the counter has reached its max
                    break;
                }
            } finally {
                lock.unlock();
            }
            // Small sleep to allow other threads to run, making the interleaving more visible
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }

    public static void main(String[] args) {
        int numThreads = 5; // Test case: 5 threads
        Thread[] threads = new Thread[numThreads];

        // Create and start threads
        for (int i = 0; i < numThreads; i++) {
            threads[i] = new Thread(Task92::accessResource);
            threads[i].start();
        }

        // Wait for all threads to complete
        for (int i = 0; i < numThreads; i++) {
            try {
                threads[i].join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        System.out.println("All threads have finished. Final counter value: " + currentCounter);

        // Reset for next test case (if any)
        // Test Case 2
        System.out.println("\n--- Test Case 2: 3 threads, maxCounter 10 ---");
        currentCounter = 0;
        maxCounter = 10;
        numThreads = 3;
        threads = new Thread[numThreads];
        for (int i = 0; i < numThreads; i++) {
            threads[i] = new Thread(Task92::accessResource);
            threads[i].start();
        }
        for (Thread t : threads) { try { t.join(); } catch (InterruptedException e) { e.printStackTrace(); } }
        System.out.println("Final counter value: " + currentCounter);


        // Test Case 3
        System.out.println("\n--- Test Case 3: 8 threads, maxCounter 50 ---");
        currentCounter = 0;
        maxCounter = 50;
        numThreads = 8;
        threads = new Thread[numThreads];
        for (int i = 0; i < numThreads; i++) {
            threads[i] = new Thread(Task92::accessResource);
            threads[i].start();
        }
        for (Thread t : threads) { try { t.join(); } catch (InterruptedException e) { e.printStackTrace(); } }
        System.out.println("Final counter value: " + currentCounter);


        // Test Case 4
        System.out.println("\n--- Test Case 4: 2 threads, maxCounter 5 ---");
        currentCounter = 0;
        maxCounter = 5;
        numThreads = 2;
        threads = new Thread[numThreads];
        for (int i = 0; i < numThreads; i++) {
            threads[i] = new Thread(Task92::accessResource);
            threads[i].start();
        }
        for (Thread t : threads) { try { t.join(); } catch (InterruptedException e) { e.printStackTrace(); } }
        System.out.println("Final counter value: " + currentCounter);


        // Test Case 5
        System.out.println("\n--- Test Case 5: 10 threads, maxCounter 10 ---");
        currentCounter = 0;
        maxCounter = 10;
        numThreads = 10;
        threads = new Thread[numThreads];
        for (int i = 0; i < numThreads; i++) {
            threads[i] = new Thread(Task92::accessResource);
            threads[i].start();
        }
        for (Thread t : threads) { try { t.join(); } catch (InterruptedException e) { e.printStackTrace(); } }
        System.out.println("Final counter value: " + currentCounter);
    }
}