import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;

public class Task95 {
    public static double getProcessorTimeSeconds() {
        ThreadMXBean bean = ManagementFactory.getThreadMXBean();
        if (bean.isThreadCpuTimeSupported() && !bean.isThreadCpuTimeEnabled()) {
            bean.setThreadCpuTimeEnabled(true);
        }
        long[] ids = bean.getAllThreadIds();
        long totalNanos = 0L;
        for (long id : ids) {
            long t = bean.getThreadCpuTime(id);
            if (t > 0) totalNanos += t;
        }
        return totalNanos / 1_000_000_000.0;
    }

    private static long burnCpu(long n) {
        long s = 0;
        for (long i = 0; i < n; i++) {
            s += i % 7;
        }
        return s;
    }

    public static void main(String[] args) {
        for (int i = 1; i <= 5; i++) {
            double secs = getProcessorTimeSeconds();
            System.out.printf("Run %d - CPU time (s): %.9f%n", i, secs);
            burnCpu(50_000_000L);
        }
    }
}