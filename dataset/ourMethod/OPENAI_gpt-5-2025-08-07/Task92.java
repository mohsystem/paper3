import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

public class Task92 {

    private static final int MAX_THREADS_CAP = 128;

    private static final class SharedState {
        private int currentCounter;
        private final int maxCounter;
        private final ReentrantLock lock = new ReentrantLock();

        SharedState(int start, int max) {
            this.currentCounter = start;
            this.maxCounter = max;
        }
    }

    private static final class Worker implements Runnable {
        private final SharedState shared;
        private final String name;

        Worker(SharedState shared, String name) {
            this.shared = shared;
            this.name = name;
        }

        @Override
        public void run() {
            while (true) {
                shared.lock.lock();
                try {
                    if (shared.currentCounter <= shared.maxCounter) {
                        shared.currentCounter += 1;
                        System.out.println("[" + name + "] incremented currentCounter to "
                                + shared.currentCounter + " (max " + shared.maxCounter + ")");
                    } else {
                        break;
                    }
                } finally {
                    shared.lock.unlock();
                }
                // Allow other threads a chance to run
                Thread.yield();
            }
        }
    }

    public static int runCounter(int start, int max, int threadCount) {
        int threads = threadCount;
        if (threads < 1) {
            return start;
        }
        if (threads > MAX_THREADS_CAP) {
            threads = MAX_THREADS_CAP;
        }

        SharedState shared = new SharedState(start, max);
        List<Thread> ts = new ArrayList<>(threads);
        for (int i = 0; i < threads; i++) {
            String workerName = "Worker-" + (i + 1);
            Thread t = new Thread(new Worker(shared, workerName), workerName);
            ts.add(t);
            t.start();
        }
        for (Thread t : ts) {
            try {
                t.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
        return shared.currentCounter;
    }

    public static void main(String[] args) {
        // 5 test cases
        int result1 = runCounter(0, 5, 3);
        System.out.println("Final counter (test1): " + result1);

        int result2 = runCounter(4, 4, 2);
        System.out.println("Final counter (test2): " + result2);

        int result3 = runCounter(10, 5, 4);
        System.out.println("Final counter (test3): " + result3);

        int result4 = runCounter(0, 50, 8);
        System.out.println("Final counter (test4): " + result4);

        int result5 = runCounter(-2, 2, 2);
        System.out.println("Final counter (test5): " + result5);
    }
}