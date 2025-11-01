
public class Lse59 {
    // Allocates a buffer on the stack and fills it with 'A'
    // Parameters:
    //   bufferSize: the size of the buffer to allocate
    // Returns: the filled buffer as a char array
    public static char[] fillBufferWithA(int bufferSize) {
        // Validate input: buffer size must be positive and within reasonable limits
        // to prevent excessive memory usage (max 10MB for stack safety)
        if (bufferSize <= 0 || bufferSize > 10 * 1024 * 1024) {
            throw new IllegalArgumentException("Buffer size must be positive and not exceed 10MB");
        }
        
        // Allocate buffer on the stack (as a local array variable)
        // In Java, local arrays are allocated on the stack for small sizes,
        // though large arrays may be heap-allocated
        char[] buffer = new char[bufferSize];
        
        // Fill buffer with 'A' - using Arrays.fill for bounds-safe operation
        // This prevents any potential buffer overflow
        java.util.Arrays.fill(buffer, 'A');
        
        return buffer;
    }
    
    public static void main(String[] args) {
        // Test case 1: Small buffer
        try {
            char[] result1 = fillBufferWithA(10);
            System.out.println("Test 1 - Size 10: " + new String(result1));
        } catch (Exception e) {
            System.out.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: Medium buffer
        try {
            char[] result2 = fillBufferWithA(100);
            System.out.println("Test 2 - Size 100: Length = " + result2.length + ", First 10 chars: " + new String(result2, 0, 10));
        } catch (Exception e) {
            System.out.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: Large buffer
        try {
            char[] result3 = fillBufferWithA(1000);
            System.out.println("Test 3 - Size 1000: Length = " + result3.length);
        } catch (Exception e) {
            System.out.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test case 4: Edge case - size 1
        try {
            char[] result4 = fillBufferWithA(1);
            System.out.println("Test 4 - Size 1: " + new String(result4));
        } catch (Exception e) {
            System.out.println("Test 4 failed: " + e.getMessage());
        }
        
        // Test case 5: Invalid input - negative size
        try {
            char[] result5 = fillBufferWithA(-1);
            System.out.println("Test 5 - Negative size should fail");
        } catch (Exception e) {
            System.out.println("Test 5 - Correctly rejected negative size: " + e.getMessage());
        }
    }
}
