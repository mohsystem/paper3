
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
        // Test case 1: 3 threads with maxCounter = 5
        System.out.println("Test Case 1:");
        currentCounter = 0;
        maxCounter = 5;
        Thread t1 = new Thread(() -> {
            for (int i = 0; i < 3; i++) {
                accessSharedResource("Thread-1");
            }
        });
        Thread t2 = new Thread(() -> {
            for (int i = 0; i < 3; i++) {
                accessSharedResource("Thread-2");
            }
        });
        Thread t3 = new Thread(() -> {
            for (int i = 0; i < 3; i++) {
                accessSharedResource("Thread-3");
            }
        });
        t1.start();
        t2.start();
        t3.start();
        try {
            t1.join();
            t2.join();
            t3.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        // Test case 2: 2 threads with maxCounter = 3
        System.out.println("\\nTest Case 2:");
        currentCounter = 0;
        maxCounter = 3;
        Thread t4 = new Thread(() -> {
            for (int i = 0; i < 2; i++) {
                accessSharedResource("Thread-4");
            }
        });
        Thread t5 = new Thread(() -> {
            for (int i = 0; i < 2; i++) {
                accessSharedResource("Thread-5");
            }
        });
        t4.start();
        t5.start();
        try {
            t4.join();
            t5.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        // Test case 3: 4 threads with maxCounter = 8
        System.out.println("\\nTest Case 3:");
        currentCounter = 0;
        maxCounter = 8;
        Thread[] threads = new Thread[4];
        for (int i = 0; i < 4; i++) {
            final int threadNum = i + 1;
            threads[i] = new Thread(() -> {
                for (int j = 0; j < 3; j++) {
                    accessSharedResource("Thread-" + threadNum);
                }
            });
            threads[i].start();
        }
        for (Thread t : threads) {
            try {
                t.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        // Test case 4: 5 threads with maxCounter = 10
        System.out.println("\\nTest Case 4:");
        currentCounter = 0;
        maxCounter = 10;
        Thread[] threads2 = new Thread[5];
        for (int i = 0; i < 5; i++) {
            final int threadNum = i + 1;
            threads2[i] = new Thread(() -> {
                for (int j = 0; j < 3; j++) {
                    accessSharedResource("Thread-" + threadNum);
                }
            });
            threads2[i].start();
        }
        for (Thread t : threads2) {
            try {
                t.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        // Test case 5: 2 threads with maxCounter = 1
        System.out.println("\\nTest Case 5:");
        currentCounter = 0;
        maxCounter = 1;
        Thread t6 = new Thread(() -> {
            accessSharedResource("Thread-6");
        });
        Thread t7 = new Thread(() -> {
            accessSharedResource("Thread-7");
        });
        t6.start();
        t7.start();
        try {
            t6.join();
            t7.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
