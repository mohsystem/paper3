import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.Semaphore;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class Task187 {

    static class H2O {
        private final Semaphore hSem;
        private final Semaphore oSem;
        private final CyclicBarrier barrier;

        public H2O() {
            hSem = new Semaphore(2);
            oSem = new Semaphore(1);

            Runnable barrierAction = () -> {
                // Reset semaphores for the next molecule
                hSem.release(2);
                oSem.release(1);
            };
            barrier = new CyclicBarrier(3, barrierAction);
        }

        public void hydrogen(Runnable releaseHydrogen) throws InterruptedException, BrokenBarrierException {
            hSem.acquire();
            barrier.await();
            releaseHydrogen.run();
        }

        public void oxygen(Runnable releaseOxygen) throws InterruptedException, BrokenBarrierException {
            oSem.acquire();
            barrier.await();
            releaseOxygen.run();
        }
    }

    public static void main(String[] args) {
        String[] testCases = {"HOH", "OOHHHH", "HHO", "HHHHHHOOO", "OHHOHH"};

        for (String water : testCases) {
            System.out.println("Input: " + water);
            runSimulation(water);
            System.out.println();
        }
    }

    private static void runSimulation(String water) {
        H2O h2o = new H2O();
        StringBuilder result = new StringBuilder();
        Object lock = new Object();

        Runnable releaseHydrogen = () -> {
            synchronized(lock) {
                result.append('H');
            }
        };
        Runnable releaseOxygen = () -> {
            synchronized(lock) {
                result.append('O');
            }
        };

        int n = water.length();
        ExecutorService executor = Executors.newFixedThreadPool(n);

        for (char c : water.toCharArray()) {
            if (c == 'H') {
                executor.submit(() -> {
                    try {
                        h2o.hydrogen(releaseHydrogen);
                    } catch (InterruptedException | BrokenBarrierException e) {
                        Thread.currentThread().interrupt();
                    }
                });
            } else { // 'O'
                executor.submit(() -> {
                    try {
                        h2o.oxygen(releaseOxygen);
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
        
        System.out.println("Output: " + result.toString());
    }
}