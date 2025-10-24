import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

public class Task92 {
    static class SharedCounter {
        final int maxCounter;
        int currentCounter = 0;
        final ReentrantLock lock = new ReentrantLock();

        SharedCounter(int maxCounter) {
            this.maxCounter = maxCounter;
        }
    }

    public static int runCounter(int maxCounter, int numThreads) {
        SharedCounter shared = new SharedCounter(maxCounter);
        List<Thread> threads = new ArrayList<>();

        Runnable worker = () -> {
            while (true) {
                shared.lock.lock();
                try {
                    if (shared.currentCounter <= shared.maxCounter) {
                        shared.currentCounter++;
                        System.out.println("Java Thread " + Thread.currentThread().getName()
                                + " incremented currentCounter to " + shared.currentCounter);
                    } else {
                        break;
                    }
                } finally {
                    shared.lock.unlock();
                }
                Thread.yield();
            }
        };

        for (int i = 0; i < numThreads; i++) {
            Thread t = new Thread(worker, "T-" + (i + 1));
            threads.add(t);
            t.start();
        }

        for (Thread t : threads) {
            try {
                t.join();
            } catch (InterruptedException ignored) {
            }
        }

        return shared.currentCounter;
    }

    public static void main(String[] args) {
        System.out.println("Java Final: " + runCounter(5, 3));
        System.out.println("Java Final: " + runCounter(0, 2));
        System.out.println("Java Final: " + runCounter(10, 5));
        System.out.println("Java Final: " + runCounter(3, 10));
        System.out.println("Java Final: " + runCounter(15, 4));
    }
}