import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;

public class Task95 {

    public static double getProcessorTimeSeconds(int mode) {
        if (mode != 0 && mode != 1) {
            throw new IllegalArgumentException("mode must be 0 or 1");
        }
        ThreadMXBean bean = ManagementFactory.getThreadMXBean();
        if (bean.isCurrentThreadCpuTimeSupported()) {
            if (!bean.isThreadCpuTimeEnabled()) {
                try {
                    bean.setThreadCpuTimeEnabled(true);
                } catch (SecurityException | UnsupportedOperationException e) {
                    // If enabling is not allowed, proceed with current state
                }
            }
            long ns = bean.getCurrentThreadCpuTime();
            if (ns < 0L) {
                return -1.0;
            }
            return ns / 1_000_000_000.0;
        } else {
            long ns = System.nanoTime();
            if (ns < 0L) {
                return -1.0;
            }
            return ns / 1_000_000_000.0;
        }
    }

    private static double burnCpu(long iterations) {
        if (iterations < 0) {
            iterations = 0;
        }
        double x = 1.0;
        for (long i = 0; i < iterations; i++) {
            x = x * 1.0000001 + 0.0000001;
            if (x > 2.0e9) {
                x = 1.0;
            }
        }
        return x;
    }

    public static void main(String[] args) {
        // 5 test cases: invoke and print processor time in seconds
        System.out.printf("%.9f%n", getProcessorTimeSeconds(0));
        double sink = burnCpu(2_000_000L);
        System.out.printf("%.9f%n", getProcessorTimeSeconds(1));
        sink += burnCpu(4_000_000L);
        System.out.printf("%.9f%n", getProcessorTimeSeconds(0));
        sink += burnCpu(1_000_000L);
        System.out.printf("%.9f%n", getProcessorTimeSeconds(1));
        // Use sink to avoid dead-code elimination
        if (sink == Double.NEGATIVE_INFINITY) {
            System.out.print("");
        }
        System.out.printf("%.9f%n", getProcessorTimeSeconds(0));
    }
}