
public class Task95 {
    public static double getCurrentProcessorTime() {
        return (double) System.nanoTime() / 1_000_000_000.0;
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
        System.out.println("\\nTest case 3 (after small delay):");
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        double time3 = getCurrentProcessorTime();
        System.out.println("Current processor time: " + time3 + " seconds");
        
        // Test case 4
        System.out.println("\\nTest case 4:");
        double time4 = getCurrentProcessorTime();
        System.out.println("Current processor time: " + time4 + " seconds");
        System.out.println("Time elapsed since test case 1: " + (time4 - time1) + " seconds");
        
        // Test case 5
        System.out.println("\\nTest case 5:");
        double time5 = getCurrentProcessorTime();
        System.out.println("Current processor time: " + time5 + " seconds");
        System.out.println("Time elapsed since test case 4: " + (time5 - time4) + " seconds");
    }
}
