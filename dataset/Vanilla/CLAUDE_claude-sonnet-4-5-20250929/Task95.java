
public class Task95 {
    public static double getCurrentProcessorTime() {
        return System.nanoTime() / 1_000_000_000.0;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test case 1:");
        double time1 = getCurrentProcessorTime();
        System.out.println("Current processor time: " + time1 + " seconds");
        
        // Test case 2
        System.out.println("\\nTest case 2:");
        double time2 = getCurrentProcessorTime();
        System.out.println("Current processor time: " + time2 + " seconds");
        
        // Test case 3 - with some delay
        System.out.println("\\nTest case 3 (after 1 second delay):");
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        double time3 = getCurrentProcessorTime();
        System.out.println("Current processor time: " + time3 + " seconds");
        
        // Test case 4
        System.out.println("\\nTest case 4:");
        double time4 = getCurrentProcessorTime();
        System.out.println("Current processor time: " + time4 + " seconds");
        System.out.println("Time difference from test 3: " + (time4 - time3) + " seconds");
        
        // Test case 5 - with computation
        System.out.println("\\nTest case 5 (after computation):");
        double startTime = getCurrentProcessorTime();
        long sum = 0;
        for (int i = 0; i < 10000000; i++) {
            sum += i;
        }
        double endTime = getCurrentProcessorTime();
        System.out.println("Start time: " + startTime + " seconds");
        System.out.println("End time: " + endTime + " seconds");
        System.out.println("Elapsed time: " + (endTime - startTime) + " seconds");
    }
}
