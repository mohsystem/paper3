import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;

public class Task95 {

    /**
     * Gets the CPU time used by the current thread in seconds.
     * @return The CPU time in seconds, or -1.0 if not supported.
     */
    public static double getProcessorTimeInSeconds() {
        ThreadMXBean bean = ManagementFactory.getThreadMXBean();
        if (bean.isCurrentThreadCpuTimeSupported()) {
            long nanoSeconds = bean.getCurrentThreadCpuTime();
            return (double) nanoSeconds / 1_000_000_000.0;
        }
        // Return a negative value to indicate that the feature is not supported.
        return -1.0;
    }

    public static void main(String[] args) {
        System.out.println("Java Processor Time Test Cases:");

        // Test Case 1: Initial time
        System.out.printf("Test 1: %.9f seconds%n", getProcessorTimeInSeconds());

        // Do some work to consume CPU time
        for (long i = 0; i < 100000000L; i++) {
            // busy loop
        }

        // Test Case 2: Time after some work
        System.out.printf("Test 2: %.9f seconds%n", getProcessorTimeInSeconds());

        // Do more intensive work
        double sum = 0;
        for (int i = 0; i < 10000000; i++) {
            sum += Math.sqrt(i);
        }

        // Test Case 3: Time after more work
        System.out.printf("Test 3: %.9f seconds%n", getProcessorTimeInSeconds());
        
        // Test Case 4: Time immediately after, should be very similar to Test 3
        System.out.printf("Test 4: %.9f seconds%n", getProcessorTimeInSeconds());

        // Do some final work
        String s = "";
        for (int i = 0; i < 20000; i++) {
            s += "a";
        }
        
        // Test Case 5: Final time
        System.out.printf("Test 5: %.9f seconds%n", getProcessorTimeInSeconds());
    }
}