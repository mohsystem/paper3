import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;

public class Task95 {

    private static volatile long SINK = 0L;

    // Returns the total CPU time of the current process in seconds, or -1.0 if unsupported.
    public static double getProcessorTimeSeconds() {
        ThreadMXBean bean = ManagementFactory.getThreadMXBean();
        if (!bean.isThreadCpuTimeSupported()) {
            return -1.0;
        }
        if (!bean.isThreadCpuTimeEnabled()) {
            try {
                bean.setThreadCpuTimeEnabled(true);
            } catch (SecurityException ignored) {
                return -1.0;
            }
        }
        long[] ids = bean.getAllThreadIds();
        long totalNs = 0L;
        for (long id : ids) {
            long t = bean.getThreadCpuTime(id);
            if (t != -1) {
                totalNs += t;
            }
        }
        return totalNs / 1_000_000_000.0;
    }

    // Minimal CPU work to advance processor time.
    private static void doCpuWork(int n) {
        if (n <= 0) return;
        long local = SINK;
        for (int i = 0; i < n; i++) {
            local += (i ^ (i << 1));
        }
        SINK = local;
    }

    public static void main(String[] args) {
        // 5 test cases
        for (int i = 1; i <= 5; i++) {
            doCpuWork(200_000 * i);
            double secs = getProcessorTimeSeconds();
            System.out.printf("Test %d: %.6f seconds%n", i, secs);
        }
    }
}