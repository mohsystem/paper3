import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;

public class Task95 {

    /**
     * Gets the current processor time for the current thread in seconds.
     *
     * @return The CPU time in seconds, or -1 if the feature is not supported.
     */
    public static double getProcessorTimeInSeconds() {
        ThreadMXBean bean = ManagementFactory.getThreadMXBean();
        if (!bean.isCurrentThreadCpuTimeSupported()) {
            return -1.0;
        }
        // getCpuTime() returns time in nanoseconds
        long nanoSeconds = bean.getCurrentThreadCpuTime();
        // Convert nanoseconds to seconds
        return nanoSeconds / 1_000_000_000.0;
    }

    // A simple function to consume some CPU time
    private static void consumeCpuTime() {
        long sum = 0;
        for (int i = 0; i < 100_000_000; i++) {
            sum += i;
        }
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases for processor time measurement in Java...");
        for (int i = 1; i <= 5; i++) {
            double startTime = getProcessorTimeInSeconds();
            System.out.printf("Test Case %d (start): %.6f seconds\n", i, startTime);
            
            // Consume some CPU time to see a change
            consumeCpuTime();
            
            double endTime = getProcessorTimeInSeconds();
            System.out.printf("Test Case %d (end):   %.6f seconds\n", i, endTime);
            System.out.println("-------------------------------------");
        }
    }
}