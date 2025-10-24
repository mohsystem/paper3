
import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;

public class Task95 {
    public static double getProcessorTimeInSeconds() {
        ThreadMXBean threadMXBean = ManagementFactory.getThreadMXBean();
        if (!threadMXBean.isCurrentThreadCpuTimeSupported()) {
            throw new UnsupportedOperationException("CPU time measurement not supported");
        }
        long cpuTimeNanos = threadMXBean.getCurrentThreadCpuTime();
        if (cpuTimeNanos < 0) {
            throw new IllegalStateException("Invalid CPU time value");
        }
        return cpuTimeNanos / 1_000_000_000.0;
    }

    public static void main(String[] args) {
        try {
            // Test case 1: Basic call
            double time1 = getProcessorTimeInSeconds();
            System.out.println("Test 1 - Processor time: " + time1 + " seconds");

            // Test case 2: After some computation
            int sum = 0;
            for (int i = 0; i < 1000000; i++) {
                sum += i;
            }
            double time2 = getProcessorTimeInSeconds();
            System.out.println("Test 2 - Processor time after computation: " + time2 + " seconds");

            // Test case 3: Multiple calls
            double time3 = getProcessorTimeInSeconds();
            System.out.println("Test 3 - Processor time: " + time3 + " seconds");

            // Test case 4: After sleep (CPU time should not increase much)
            Thread.sleep(10);
            double time4 = getProcessorTimeInSeconds();
            System.out.println("Test 4 - Processor time after sleep: " + time4 + " seconds");

            // Test case 5: Final measurement
            double time5 = getProcessorTimeInSeconds();
            System.out.println("Test 5 - Processor time: " + time5 + " seconds");

        } catch (UnsupportedOperationException | IllegalStateException e) {
            System.err.println("Error: " + e.getMessage());
        } catch (InterruptedException e) {
            System.err.println("Thread interrupted");
            Thread.currentThread().interrupt();
        }
    }
}
