
import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;

public class Task95 {
    public static double getProcessorTimeInSeconds() {
        ThreadMXBean threadMXBean = ManagementFactory.getThreadMXBean();
        long cpuTime = threadMXBean.getCurrentThreadCpuTime();
        return cpuTime / 1_000_000_000.0;
    }
    
    public static void printProcessorTime() {
        double processorTime = getProcessorTimeInSeconds();
        System.out.println("Current processor time: " + processorTime + " seconds");
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        printProcessorTime();
        
        System.out.println("\\nTest Case 2:");
        for (int i = 0; i < 1000000; i++) {
            Math.sqrt(i);
        }
        printProcessorTime();
        
        System.out.println("\\nTest Case 3:");
        for (int i = 0; i < 5000000; i++) {
            Math.sin(i);
        }
        printProcessorTime();
        
        System.out.println("\\nTest Case 4:");
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        printProcessorTime();
        
        System.out.println("\\nTest Case 5:");
        for (int i = 0; i < 10000000; i++) {
            String s = String.valueOf(i);
        }
        printProcessorTime();
    }
}
