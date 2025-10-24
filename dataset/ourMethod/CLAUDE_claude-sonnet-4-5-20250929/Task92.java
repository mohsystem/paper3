
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.atomic.AtomicInteger;

public class Task92 {
    private static final Lock lock = new ReentrantLock();
    private static final AtomicInteger currentCounter = new AtomicInteger(0);
    private static int maxCounter;

    public static boolean accessSharedResource(String threadName, int max) {
        if (threadName == null || threadName.isEmpty()) {
            return false;
        }
        if (max < 0) {
            return false;
        }
        
        int current = currentCounter.get();
        if (current <= max) {
            lock.lock();
            try {
                current = currentCounter.get();
                if (current <= max) {
                    currentCounter.incrementAndGet();
                    System.out.println("Thread " + threadName + " is accessing currentCounter: " + currentCounter.get());
                    return true;
                }
            } finally {
                lock.unlock();
            }
        }
        return false;
    }

    public static void resetCounter() {
        lock.lock();
        try {
            currentCounter.set(0);
        } finally {
            lock.unlock();
        }
    }

    public static int getCurrentCounter() {
        return currentCounter.get();
    }

    public static void main(String[] args) {
        maxCounter = 5;
        
        System.out.println("Test Case 1: Multiple threads with maxCounter=5");
        resetCounter();
        Thread t1 = new Thread(() -> accessSharedResource("T1", 5));
        Thread t2 = new Thread(() -> accessSharedResource("T2", 5));
        Thread t3 = new Thread(() -> accessSharedResource("T3", 5));
        t1.start();
        t2.start();
        t3.start();
        try {
            t1.join();
            t2.join();
            t3.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        System.out.println("Final counter: " + getCurrentCounter() + "\\n");

        System.out.println("Test Case 2: Threads exceeding maxCounter");
        resetCounter();
        for (int i = 0; i < 8; i++) {
            final int num = i;
            new Thread(() -> accessSharedResource("T" + num, 5)).start();
        }
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        System.out.println("Final counter: " + getCurrentCounter() + "\\n");

        System.out.println("Test Case 3: maxCounter=0");
        resetCounter();
        accessSharedResource("T_Zero", 0);
        System.out.println("Final counter: " + getCurrentCounter() + "\\n");

        System.out.println("Test Case 4: Sequential access");
        resetCounter();
        for (int i = 0; i < 3; i++) {
            accessSharedResource("T_Seq" + i, 10);
        }
        System.out.println("Final counter: " + getCurrentCounter() + "\\n");

        System.out.println("Test Case 5: Invalid inputs");
        resetCounter();
        accessSharedResource(null, 5);
        accessSharedResource("", 5);
        accessSharedResource("T_Valid", -1);
        System.out.println("Final counter: " + getCurrentCounter());
    }
}
