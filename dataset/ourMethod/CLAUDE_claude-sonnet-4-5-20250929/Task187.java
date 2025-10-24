
import java.util.concurrent.Semaphore;
import java.util.concurrent.locks.ReentrantLock;

class Task187 {
    private Semaphore hSem;
    private Semaphore oSem;
    private ReentrantLock lock;
    private int hCount;
    private int oCount;
    private StringBuilder result;

    public Task187() {
        hSem = new Semaphore(2);
        oSem = new Semaphore(1);
        lock = new ReentrantLock();
        hCount = 0;
        oCount = 0;
        result = new StringBuilder();
    }

    public void hydrogen(Runnable releaseHydrogen) throws InterruptedException {
        hSem.acquire();
        lock.lock();
        try {
            releaseHydrogen.run();
            hCount++;
            if (hCount == 2 && oCount == 1) {
                hCount = 0;
                oCount = 0;
                oSem.release();
                hSem.release(2);
            }
        } finally {
            lock.unlock();
        }
    }

    public void oxygen(Runnable releaseOxygen) throws InterruptedException {
        oSem.acquire();
        lock.lock();
        try {
            releaseOxygen.run();
            oCount++;
            if (hCount == 2 && oCount == 1) {
                hCount = 0;
                oCount = 0;
                oSem.release();
                hSem.release(2);
            }
        } finally {
            lock.unlock();
        }
    }

    public String processWater(String water) {
        if (water == null || water.length() == 0 || water.length() % 3 != 0) {
            return "";
        }
        
        result = new StringBuilder();
        Thread[] threads = new Thread[water.length()];
        
        for (int i = 0; i < water.length(); i++) {
            final char c = water.charAt(i);
            if (c != 'H' && c != 'O') {
                return "";
            }
            
            threads[i] = new Thread(() -> {
                try {
                    if (c == 'H') {
                        hydrogen(() -> {
                            synchronized (result) {
                                result.append('H');
                            }
                        });
                    } else {
                        oxygen(() -> {
                            synchronized (result) {
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
        Task187 t1 = new Task187();
        System.out.println("Test 1: " + t1.processWater("HOH"));
        
        Task187 t2 = new Task187();
        System.out.println("Test 2: " + t2.processWater("OOHHHH"));
        
        Task187 t3 = new Task187();
        System.out.println("Test 3: " + t3.processWater("HHHOOO"));
        
        Task187 t4 = new Task187();
        System.out.println("Test 4: " + t4.processWater("HHO"));
        
        Task187 t5 = new Task187();
        System.out.println("Test 5: " + t5.processWater("HHHHHHOOO"));
    }
}
