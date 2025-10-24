// Chain-of-Through Step 1: Problem understanding
// - Implement a function that returns current processor (CPU) time in seconds.
// - Provide a main class Task95 with 5 test cases printing the value.
//
// Chain-of-Through Step 2: Security requirements
// - Avoid unsafe operations and external inputs.
// - Use standard APIs; handle absence of CPU-time support gracefully.
//
// Chain-of-Through Step 3: Secure coding generation
// - Use ThreadMXBean for thread CPU time; if unavailable, try process CPU time via reflection.
// - Do not rely on privileged operations; catch and handle exceptions.
//
// Chain-of-Through Step 4: Code review
// - No external input, no sensitive operations, exceptions handled safely.
//
// Chain-of-Through Step 5: Secure code output
// - Final code below.

import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;

public class Task95 {

    // Returns current processor (CPU) time in seconds for the current thread or process if available.
    public static double getProcessorTimeSeconds() {
        try {
            ThreadMXBean bean = ManagementFactory.getThreadMXBean();
            if (bean.isThreadCpuTimeSupported()) {
                if (!bean.isThreadCpuTimeEnabled()) {
                    try {
                        bean.setThreadCpuTimeEnabled(true);
                    } catch (SecurityException se) {
                        // Cannot enable; continue to attempt retrieval if still available
                    }
                }
                long ns = bean.getCurrentThreadCpuTime();
                if (ns >= 0L) {
                    return ns / 1_000_000_000.0;
                }
            }
        } catch (SecurityException ignored) {
            // Fall through to other methods
        }

        // Fallback: Try process CPU time via com.sun.management.OperatingSystemMXBean if available.
        try {
            Object osBean = ManagementFactory.getOperatingSystemMXBean();
            Class<?> sunOsBeanClass = Class.forName("com.sun.management.OperatingSystemMXBean");
            if (sunOsBeanClass.isInstance(osBean)) {
                java.lang.reflect.Method m = sunOsBeanClass.getMethod("getProcessCpuTime");
                Object val = m.invoke(osBean);
                if (val instanceof Long) {
                    long ns = (Long) val;
                    if (ns >= 0L) {
                        return ns / 1_000_000_000.0;
                    }
                }
            }
        } catch (Throwable ignored) {
            // Reflection/class not available; return sentinel value
        }

        // If CPU time is not supported, return -1.0 to indicate unavailability.
        return -1.0;
    }

    // Simple CPU burn to make CPU time advance between tests
    private static volatile double sink = 0.0;
    public static void burnCpu(long iterations) {
        double x = 0.0;
        for (long i = 1; i <= iterations; i++) {
            x += Math.sqrt((i % 1000) + 1.0);
        }
        sink = x; // prevent optimization
    }

    public static void main(String[] args) {
        // 5 test cases: call the function and print the processor time in seconds
        for (int t = 1; t <= 5; t++) {
            burnCpu(2_000_00L * t); // increasing work to show time progression
            double seconds = getProcessorTimeSeconds();
            System.out.printf("Java Test %d - CPU Time (s): %.9f%n", t, seconds);
        }
    }
}