
import java.util.concurrent.Semaphore;

class Task187 {
    private Semaphore hydrogenSemaphore;
    private Semaphore oxygenSemaphore;
    private Semaphore mutex;
    private int hydrogenCount;

    public Task187() {
        hydrogenSemaphore = new Semaphore(2);
        oxygenSemaphore = new Semaphore(0);
        mutex = new Semaphore(1);
        hydrogenCount = 0;
    }

    public void hydrogen(Runnable releaseHydrogen) throws InterruptedException {
        hydrogenSemaphore.acquire();
        
        mutex.acquire();
        hydrogenCount++;
        if (hydrogenCount == 2) {
            oxygenSemaphore.release();
        }
        mutex.release();
        
        releaseHydrogen.run();
    }

    public void oxygen(Runnable releaseOxygen) throws InterruptedException {
        oxygenSemaphore.acquire();
        
        releaseOxygen.run();
        
        mutex.acquire();
        hydrogenCount = 0;
        mutex.release();
        
        hydrogenSemaphore.release(2);
    }

    public static void main(String[] args) {
        String[] testCases = {"HOH", "OOHHHH", "HHOHHO", "HHHHHHOOO", "OHHHOH"};
        
        for (String water : testCases) {
            System.out.println("Input: " + water);
            Task187 h2o = new Task187();
            StringBuilder result = new StringBuilder();
            
            Thread[] threads = new Thread[water.length()];
            for (int i = 0; i < water.length(); i++) {
                final char c = water.charAt(i);
                threads[i] = new Thread(() -> {
                    try {
                        if (c == 'H') {
                            h2o.hydrogen(() -> {
                                synchronized (result) {
                                    result.append('H');
                                }
                            });
                        } else {
                            h2o.oxygen(() -> {
                                synchronized (result) {
                                    result.append('O');
                                }
                            });
                        }
                    } catch (InterruptedException e) {
                        e.printStackTrace();
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
            
            System.out.println("Output: " + result.toString());
            System.out.println();
        }
    }
}
