
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class Task92 {
    private static int currentCounter = 0;
    private static int maxCounter = 10;
    private static final Lock lock = new ReentrantLock();

    public static void accessSharedResource(String threadName) {
        if (currentCounter <= maxCounter) {
            lock.lock();
            try {
                // Double-check after acquiring lock
                if (currentCounter <= maxCounter) {
                    currentCounter++;
                    System.out.println(threadName + " is accessing currentCounter: " + currentCounter);
                }
            } finally {
                lock.unlock();
            }
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Multiple threads with counter limit 10
        currentCounter = 0;
        maxCounter = 10;
        System.out.println("Test Case 1: maxCounter = 10");
        Thread[] threads1 = new Thread[5];
        for (int i = 0; i < 5; i++) {
            final String name = "Thread-" + (i + 1);
            threads1[i] = new Thread(() -> {
                for (int j = 0; j < 3; j++) {
                    accessSharedResource(name);
                }
            });
            threads1[i].start();
        }
        for (Thread t : threads1) {
            try { t.join(); } catch (InterruptedException e) { e.printStackTrace(); }
        }

        // Test Case 2: Counter limit 5
        currentCounter = 0;
        maxCounter = 5;
        System.out.println("\\nTest Case 2: maxCounter = 5");
        Thread[] threads2 = new Thread[3];
        for (int i = 0; i < 3; i++) {
            final String name = "Thread-" + (i + 1);
            threads2[i] = new Thread(() -> {
                for (int j = 0; j < 2; j++) {
                    accessSharedResource(name);
                }
            });
            threads2[i].start();
        }
        for (Thread t : threads2) {
            try { t.join(); } catch (InterruptedException e) { e.printStackTrace(); }
        }

        // Test Case 3: Counter limit 0
        currentCounter = 0;
        maxCounter = 0;
        System.out.println("\\nTest Case 3: maxCounter = 0");
        Thread t3 = new Thread(() -> accessSharedResource("Thread-1"));
        t3.start();
        try { t3.join(); } catch (InterruptedException e) { e.printStackTrace(); }

        // Test Case 4: Counter limit 20
        currentCounter = 0;
        maxCounter = 20;
        System.out.println("\\nTest Case 4: maxCounter = 20");
        Thread[] threads4 = new Thread[10];
        for (int i = 0; i < 10; i++) {
            final String name = "Thread-" + (i + 1);
            threads4[i] = new Thread(() -> {
                for (int j = 0; j < 2; j++) {
                    accessSharedResource(name);
                }
            });
            threads4[i].start();
        }
        for (Thread t : threads4) {
            try { t.join(); } catch (InterruptedException e) { e.printStackTrace(); }
        }

        // Test Case 5: Single thread with counter limit 3
        currentCounter = 0;
        maxCounter = 3;
        System.out.println("\\nTest Case 5: Single thread, maxCounter = 3");
        Thread t5 = new Thread(() -> {
            for (int i = 0; i < 5; i++) {
                accessSharedResource("Thread-Single");
            }
        });
        t5.start();
        try { t5.join(); } catch (InterruptedException e) { e.printStackTrace(); }
    }
}
