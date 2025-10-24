import java.util.ArrayList;
import java.util.List;

public class Task92 {

    private static int currentCounter;
    private static int maxCounter;
    private static final Object lock = new Object();

    // The runnable task for each thread
    static class CounterIncrementer implements Runnable {
        @Override
        public void run() {
            while (true) {
                // Acquire lock
                synchronized (lock) {
                    // Check if the counter is less than the max value
                    if (currentCounter < maxCounter) {
                        // Increment counter and print
                        currentCounter++;
                        System.out.println(Thread.currentThread().getName() + " is accessing counter: " + currentCounter);
                    } else {
                        // Max count reached, exit the loop
                        break;
                    }
                } // The lock is released automatically when the synchronized block is exited
            }
        }
    }

    /**
     * Sets up and runs a single simulation with a given number of threads and a max counter value.
     * @param numThreads The number of threads to create.
     * @param maxCount The maximum value for the counter.
     */
    public static void runSimulation(int numThreads, int maxCount) {
        System.out.println("--- Starting Test: " + numThreads + " threads, maxCounter = " + maxCount + " ---");
        
        // Reset shared state for the test
        currentCounter = 0;
        maxCounter = maxCount;
        
        List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < numThreads; i++) {
            // Create and start each thread
            Thread t = new Thread(new CounterIncrementer(), "Thread-" + (i + 1));
            threads.add(t);
            t.start();
        }
        
        // Wait for all threads to complete their execution
        for (Thread t : threads) {
            try {
                t.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                System.err.println("Thread interrupted: " + e.getMessage());
            }
        }
        
        System.out.println("--- Test Finished. Final counter value: " + currentCounter + " ---\n");
    }

    public static void main(String[] args) {
        // 5 test cases as required
        runSimulation(2, 10);
        runSimulation(5, 20);
        runSimulation(10, 100);
        runSimulation(3, 5);
        runSimulation(8, 50);
    }
}