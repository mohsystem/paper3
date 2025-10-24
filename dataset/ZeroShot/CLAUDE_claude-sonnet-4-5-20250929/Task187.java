
import java.util.concurrent.Semaphore;
import java.util.concurrent.locks.ReentrantLock;

class Task187 {
    private Semaphore hydrogenSem;
    private Semaphore oxygenSem;
    private ReentrantLock lock;
    private int hydrogenCount;
    private StringBuilder result;

    public Task187() {
        hydrogenSem = new Semaphore(2);
        oxygenSem = new Semaphore(0);
        lock = new ReentrantLock();
        hydrogenCount = 0;
        result = new StringBuilder();
    }

    public void hydrogen(Runnable releaseHydrogen) throws InterruptedException {
        hydrogenSem.acquire();
        lock.lock();
        try {
            releaseHydrogen.run();
            hydrogenCount++;
            if (hydrogenCount == 2) {
                oxygenSem.release();
            }
        } finally {
            lock.unlock();
        }
    }

    public void oxygen(Runnable releaseOxygen) throws InterruptedException {
        oxygenSem.acquire();
        lock.lock();
        try {
            releaseOxygen.run();
            hydrogenCount = 0;
            hydrogenSem.release(2);
        } finally {
            lock.unlock();
        }
    }

    public String buildWater(String water) {
        result = new StringBuilder();
        Thread[] threads = new Thread[water.length()];
        
        for (int i = 0; i < water.length(); i++) {
            final char c = water.charAt(i);
            threads[i] = new Thread(() -> {
                try {
                    if (c == 'H') {
                        hydrogen(() -> {
                            synchronized(result) {
                                result.append('H');
                            }
                        });
                    } else {
                        oxygen(() -> {
                            synchronized(result) {
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
        
        return result.toString();
    }

    public static void main(String[] args) {
        String[] testCases = {"HOH", "OOHHHH", "HHOHHO", "HHHHHHOOO", "OHHHOH"};
        
        for (String testCase : testCases) {
            Task187 h2o = new Task187();
            String output = h2o.buildWater(testCase);
            System.out.println("Input: " + testCase + " -> Output: " + output);
        }
    }
}
