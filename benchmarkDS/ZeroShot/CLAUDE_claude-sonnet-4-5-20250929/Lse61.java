
// WARNING: This code contains a critical security vulnerability (double-free)
// For educational purposes only - DO NOT USE IN PRODUCTION

public class Lse61 {
    public static void demonstrateVulnerability(int bufferSize, int freeCount) {
        // Note: Java has automatic garbage collection, so this vulnerability
        // cannot be directly demonstrated as in C/C++. 
        // Java doesn't allow manual memory freeing like C's free()
        
        // Creating a buffer
        byte[] buffer = new byte[bufferSize];
        
        System.out.println("Buffer allocated with size: " + bufferSize);
        
        // In Java, we can only set reference to null
        // The garbage collector will handle deallocation
        for (int i = 0; i < freeCount; i++) {
            buffer = null; // Setting to null (not true "freeing")
            System.out.println("Buffer reference set to null, iteration: " + (i + 1));
        }
        
        System.out.println("Note: Java uses garbage collection, so double-free vulnerabilities don't exist as in C/C++");
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        demonstrateVulnerability(1024, 3);
        
        System.out.println("\\nTest Case 2:");
        demonstrateVulnerability(2048, 5);
        
        System.out.println("\\nTest Case 3:");
        demonstrateVulnerability(512, 2);
        
        System.out.println("\\nTest Case 4:");
        demonstrateVulnerability(4096, 1);
        
        System.out.println("\\nTest Case 5:");
        demonstrateVulnerability(256, 10);
    }
}
