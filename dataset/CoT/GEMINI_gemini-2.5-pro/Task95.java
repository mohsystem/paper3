import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;

public class Task95 {

    /**
     * Gets the current thread's CPU time in seconds.
     * Note: This measures the CPU time for the current thread, which is a good
     * approximation for the processor time of a single-threaded application.
     * @return The CPU time in seconds as a double, or 0.0 if not supported.
     */
    public static double getProcessorTimeInSeconds() {
        ThreadMXBean mxBean = ManagementFactory.getThreadMXBean();
        // Check if CPU time measurement is supported on the current JVM
        if (mxBean.isCurrentThreadCpuTimeSupported()) {
            // Get time in nanoseconds and convert to seconds
            long nanoSeconds = mxBean.getCurrentThreadCpuTime();
            return nanoSeconds / 1_000_000_000.0;
        }
        // Return 0.0 or throw an exception if CPU time is not supported
        return 0.0;
    }
    
    /**
     * A simple function to consume some CPU time to make the measurements meaningful.
     */
    private static void doSomeWork() {
        long sum = 0;
        for (long i = 0; i < 100_000_000L; i++) {
            sum += i;
        }
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases to measure processor time...");

        // Test Case 1: Initial measurement
        System.out.printf("Test Case 1: Start time: %.6f seconds%n", getProcessorTimeInSeconds());
        
        // Test Case 2: After some work
        doSomeWork();
        System.out.printf("Test Case 2: After first work: %.6f seconds%n", getProcessorTimeInSeconds());
        
        // Test Case 3: After more work
        doSomeWork();
        System.out.printf("Test Case 3: After second work: %.6f seconds%n", getProcessorTimeInSeconds());
        
        // Test Case 4: A measurement immediately after the previous one
        System.out.printf("Test Case 4: Immediately after: %.6f seconds%n", getProcessorTimeInSeconds());
        
        // Test Case 5: After a final round of work
        doSomeWork();
        System.out.printf("Test Case 5: After final work: %.6f seconds%n", getProcessorTimeInSeconds());
    }
}