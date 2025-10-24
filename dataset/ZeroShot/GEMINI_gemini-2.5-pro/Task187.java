import java.util.concurrent.Semaphore;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

class H2O {
    private final Semaphore hSem = new Semaphore(0);
    private final Semaphore oSem = new Semaphore(0);
    private final Lock lock = new ReentrantLock();
    private int hAtomsWaiting = 0;

    public H2O() {}

    public void hydrogen(Runnable releaseHydrogen) throws InterruptedException {
        lock.lock();
        try {
            hAtomsWaiting++;
            if (hAtomsWaiting >= 2) {
                // A pair of H atoms is ready. Wake them up along with an O atom.
                hSem.release(2);
                hAtomsWaiting -= 2;
                oSem.release(1);
            }
        } finally {
            lock.unlock();
        }
        
        hSem.acquire();
        releaseHydrogen.run();
    }

    public void oxygen(Runnable releaseOxygen) throws InterruptedException {
        oSem.acquire();
        releaseOxygen.run();
    }
}

public class Task187 {
    public static void main(String[] args) {
        String[] testCases = {"HOH", "OOHHHH", "HHHHOO", "HOOHH", "HHHHHHOOO"};

        for (String testCase : testCases) {
            System.out.println("Testing with input: " + testCase);
            StringBuilder output = new StringBuilder();
            H2O h2o = new H2O();
            
            Thread[] threads = new Thread[testCase.length()];
            for (int i = 0; i < testCase.length(); i++) {
                final char type = testCase.charAt(i);
                if (type == 'H') {
                    threads[i] = new Thread(() -> {
                        try {
                            h2o.hydrogen(() -> {
                                synchronized(output) {
                                    output.append('H');
                                }
                            });
                        } catch (InterruptedException e) {
                            Thread.currentThread().interrupt();
                        }
                    });
                } else { // 'O'
                    threads[i] = new Thread(() -> {
                        try {
                            h2o.oxygen(() -> {
                                synchronized(output) {
                                    output.append('O');
                                }
                            });
                        } catch (InterruptedException e) {
                            Thread.currentThread().interrupt();
                        }
                    });
                }
            }

            for (Thread t : threads) {
                t.start();
            }

            for (Thread t : threads) {
                try {
                    t.join();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            }
            System.out.println("Output: " + output.toString());
            System.out.println("--------------------");
        }
    }
}