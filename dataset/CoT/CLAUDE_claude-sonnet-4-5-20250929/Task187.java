
import java.util.concurrent.Semaphore;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

class Task187 {
    private Semaphore hydrogenSemaphore;
    private Semaphore oxygenSemaphore;
    private Lock lock;
    private int hydrogenCount;
    
    public Task187() {
        hydrogenSemaphore = new Semaphore(2);
        oxygenSemaphore = new Semaphore(0);
        lock = new ReentrantLock();
        hydrogenCount = 0;
    }
    
    public void hydrogen(Runnable releaseHydrogen) throws InterruptedException {
        hydrogenSemaphore.acquire();
        lock.lock();
        try {
            releaseHydrogen.run();
            hydrogenCount++;
            if (hydrogenCount == 2) {
                oxygenSemaphore.release();
                hydrogenCount = 0;
            }
        } finally {
            lock.unlock();
        }
    }
    
    public void oxygen(Runnable releaseOxygen) throws InterruptedException {
        oxygenSemaphore.acquire();
        lock.lock();
        try {
            releaseOxygen.run();
            hydrogenSemaphore.release(2);
        } finally {
            lock.unlock();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        testCase("HOH");
        
        // Test case 2
        testCase("OOHHHH");
        
        // Test case 3
        testCase("HHHHHHOOO");
        
        // Test case 4
        testCase("HHHOHHHOOO");
        
        // Test case 5
        testCase("HHOOHH");
    }
    
    private static void testCase(String water) {
        Task187 h2o = new Task187();
        StringBuilder result = new StringBuilder();
        Object lock = new Object();
        
        Thread[] threads = new Thread[water.length()];
        
        for (int i = 0; i < water.length(); i++) {
            final char c = water.charAt(i);
            threads[i] = new Thread(() -> {
                try {
                    if (c == 'H') {
                        h2o.hydrogen(() -> {
                            synchronized (lock) {
                                result.append('H');
                            }
                        });
                    } else {
                        h2o.oxygen(() -> {
                            synchronized (lock) {
                                result.append('O');
                            }
                        });
                    }
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            });
            threads[i].start();
        }
        
        for (Thread thread : threads) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
        
        System.out.println("Input: " + water + " -> Output: " + result.toString());
    }
}
