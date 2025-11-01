
public class Lse61 {
    // Demonstrates proper memory management pattern
    // Allocates and deallocates buffer correctly - no double-free
    public static void allocateAndFreeBuffer(int bufferSize, int iterations) {
        // Validate input to prevent negative or excessive allocations
        if (bufferSize <= 0 || bufferSize > 1024 * 1024 * 100) { // Max 100MB
            throw new IllegalArgumentException("Invalid buffer size");
        }
        if (iterations <= 0 || iterations > 1000) {
            throw new IllegalArgumentException("Invalid iteration count");
        }
        
        // Correct pattern: allocate and free within each iteration
        // This prevents double-free vulnerability
        for (int i = 0; i < iterations; i++) {
            // Allocate buffer for this iteration
            byte[] buffer = new byte[bufferSize];
            
            // Use buffer (example: initialize to demonstrate allocation worked)
            if (buffer.length > 0) {
                buffer[0] = (byte) i;
            }
            
            // In Java, explicit free not needed - GC handles it
            // Setting to null helps GC in tight loops
            buffer = null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Small buffer, few iterations
        allocateAndFreeBuffer(1024, 5);
        System.out.println("Test 1 passed: 1KB buffer, 5 iterations");
        
        // Test case 2: Medium buffer, moderate iterations
        allocateAndFreeBuffer(8192, 10);
        System.out.println("Test 2 passed: 8KB buffer, 10 iterations");
        
        // Test case 3: Larger buffer, single iteration
        allocateAndFreeBuffer(65536, 1);
        System.out.println("Test 3 passed: 64KB buffer, 1 iteration");
        
        // Test case 4: Small buffer, many iterations
        allocateAndFreeBuffer(512, 100);
        System.out.println("Test 4 passed: 512B buffer, 100 iterations");
        
        // Test case 5: Edge case - minimum valid size
        allocateAndFreeBuffer(1, 3);
        System.out.println("Test 5 passed: 1B buffer, 3 iterations");
    }
}
