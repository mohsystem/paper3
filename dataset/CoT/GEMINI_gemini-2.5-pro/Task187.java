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
        // Semaphore for hydrogen allows 2 permits.
        hSem = new Semaphore(2);
        // Semaphore for oxygen allows 1 permit.
        oSem = new Semaphore(1);

        // A barrier for 3 threads (2H + 1O). When the barrier is tripped,
        // it resets the semaphores for the next molecule.
        barrier = new CyclicBarrier(3, () -> {
            hSem.release(2);
            oSem.release(1);
        });
    }

    public void hydrogen(Runnable releaseHydrogen) throws InterruptedException, BrokenBarrierException {
        hSem.acquire();
        // releaseHydrogen.run() outputs "H".
        releaseHydrogen.run();
        barrier.await();
    }

    public void oxygen(Runnable releaseOxygen) throws InterruptedException, BrokenBarrierException {
        oSem.acquire();
        // releaseOxygen.run() outputs "O".
        releaseOxygen.run();
        barrier.await();
    }
}

public class Task187 {

    public static String runWaterSimulation(String water) {
        H2O h2o = new H2O();
        StringBuilder result = new StringBuilder();
        ExecutorService executor = Executors.newFixedThreadPool(water.length());

        for (char atom : water.toCharArray()) {
            if (atom == 'H') {
                executor.submit(() -> {
                    try {
                        h2o.hydrogen(() -> result.append('H'));
                    } catch (InterruptedException | BrokenBarrierException e) {
                        Thread.currentThread().interrupt();
                    }
                });
            } else if (atom == 'O') {
                 executor.submit(() -> {
                    try {
                        h2o.oxygen(() -> result.append('O'));
                    } catch (InterruptedException | BrokenBarrierException e) {
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

        return result.toString();
    }

    public static void main(String[] args) {
        String[] testCases = {
            "HOH",
            "OOHHHH",
            "HHOHHO",
            "HHHHHHOOO",
            "OHHOHHOHH"
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input: " + testCases[i]);
            String output = runWaterSimulation(testCases[i]);
            System.out.println("Output: " + output);
            System.out.println("--------------------");
        }
    }
}