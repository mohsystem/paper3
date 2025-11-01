
public class Lse61 {
    public static void allocateAndFreeBuffer(int bufferSize, int iterations) {
        for (int i = 0; i < iterations; i++) {
            byte[] buffer = new byte[bufferSize];
            // In Java, explicit deallocation is not needed
            // The garbage collector will handle memory cleanup
            buffer = null;
            System.gc(); // Suggest garbage collection (not guaranteed to run immediately)
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Small buffer, few iterations
        System.out.println("Test 1: Buffer size 1024, 5 iterations");
        allocateAndFreeBuffer(1024, 5);
        
        // Test case 2: Medium buffer, moderate iterations
        System.out.println("Test 2: Buffer size 4096, 10 iterations");
        allocateAndFreeBuffer(4096, 10);
        
        // Test case 3: Large buffer, few iterations
        System.out.println("Test 3: Buffer size 1048576, 3 iterations");
        allocateAndFreeBuffer(1048576, 3);
        
        // Test case 4: Small buffer, many iterations
        System.out.println("Test 4: Buffer size 512, 20 iterations");
        allocateAndFreeBuffer(512, 20);
        
        // Test case 5: Very small buffer, moderate iterations
        System.out.println("Test 5: Buffer size 128, 15 iterations");
        allocateAndFreeBuffer(128, 15);
        
        System.out.println("All tests completed");
    }
}
