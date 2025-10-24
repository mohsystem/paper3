import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

class H2O {

    private final Semaphore hSem;
    private final Semaphore oSem;
    private final CyclicBarrier barrier;

    public H2O() {
        hSem = new Semaphore(2);
        oSem = new Semaphore(1);
        
        // This action will run when the barrier is tripped (i.e., when 3 threads arrive).
        // It resets the semaphores for the next molecule.
        Runnable barrierAction = () -> {
            hSem.release(2);
            oSem.release(1);
        };
        barrier = new CyclicBarrier(3, barrierAction);
    }

    public void hydrogen(Runnable releaseHydrogen) throws InterruptedException {
        hSem.acquire();
        try {
            barrier.await();
        } catch (BrokenBarrierException e) {
            Thread.currentThread().interrupt();
        }
        // releaseHydrogen.run() outputs "H". Do not change or remove this line.
        releaseHydrogen.run();
    }

    public void oxygen(Runnable releaseOxygen) throws InterruptedException {
        oSem.acquire();
        try {
            barrier.await();
        } catch (BrokenBarrierException e) {
            Thread.currentThread().interrupt();
        }
        // releaseOxygen.run() outputs "O". Do not change or remove this line.
        releaseOxygen.run();
    }
}

public class Task187 {
    public static void runTest(String input) {
        H2O h2o = new H2O();
        StringBuilder sb = new StringBuilder();
        ExecutorService executor = Executors.newFixedThreadPool(input.length());

        for (char c : input.toCharArray()) {
            if (c == 'H') {
                executor.submit(() -> {
                    try {
                        h2o.hydrogen(() -> sb.append('H'));
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                });
            } else if (c == 'O') {
                executor.submit(() -> {
                    try {
                        h2o.oxygen(() -> sb.append('O'));
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                });
            }
        }

        executor.shutdown();
        try {
            if (!executor.awaitTermination(5, TimeUnit.SECONDS)) {
                executor.shutdownNow();
            }
        } catch (InterruptedException e) {
            executor.shutdownNow();
        }
        
        System.out.println("Input: " + input);
        System.out.println("Output: " + sb.toString());
        System.out.println("---");
    }

    public static void main(String[] args) {
        String[] testCases = {
            "HOH",
            "OOHHHH",
            "HHHHHHOOO",
            "HOHOHH",
            "OOOHHHHHH"
        };

        for (String testCase : testCases) {
            runTest(testCase);
        }
    }
}