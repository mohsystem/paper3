import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;

public class Task95 {

    /**
     * Gets the current processor time used by the current thread in seconds.
     * Note: This measures CPU time for the current thread, not the entire process.
     *
     * @return The CPU time in seconds, or -1.0 if it's not supported.
     */
    public static double getProcessorTimeInSeconds() {
        try {
            ThreadMXBean bean = ManagementFactory.getThreadMXBean();
            if (bean.isCurrentThreadCpuTimeSupported()) {
                long nanoTime = bean.getCurrentThreadCpuTime();
                return (double) nanoTime / 1_000_000_000.0;
            }
        } catch (UnsupportedOperationException e) {
            // This exception can be thrown if the JVM does not support CPU time measurement.
            System.err.println("Error getting processor time: " + e.getMessage());
        }
        return -1.0;
    }

    /**
     * A simple function to consume some CPU time.
     * The volatile keyword is used to ensure the loop is not optimized away.
     */
    private static void consumeCpuTime() {
        volatile long sum = 0;
        for (int i = 0; i < 100_000_000; i++) {
            sum += i;
        }
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases for getting processor time:");

        for (int i = 1; i <= 5; i++) {
            System.out.println("\n--- Test Case " + i + " ---");
            
            double startTime = getProcessorTimeInSeconds();
            if (startTime < 0) {
                System.out.println("Processor time measurement is not supported on this platform.");
                break;
            }
            System.out.printf("Initial processor time: %.6f seconds%n", startTime);

            // Consume some CPU time
            consumeCpuTime();

            double endTime = getProcessorTimeInSeconds();
            if (endTime < 0) {
                 System.out.println("Processor time measurement is not supported on this platform.");
                 break;
            }
            System.out.printf("Processor time after work: %.6f seconds%n", endTime);
            System.out.printf("Elapsed processor time: %.6f seconds%n", endTime - startTime);
        }
    }
}