
public class Lse59 {
    private static final int MAX_BUFFER_SIZE = 1024; // Limit to prevent stack overflow
    
    public static String allocateAndFillBuffer(int bufferSize) {
        // Validate input
        if (bufferSize <= 0) {
            throw new IllegalArgumentException("Buffer size must be positive");
        }
        if (bufferSize > MAX_BUFFER_SIZE) {
            throw new IllegalArgumentException("Buffer size exceeds maximum allowed size");
        }
        
        // Allocate buffer on stack (local variable)
        char[] buffer = new char[bufferSize];
        
        // Fill buffer with 'A'
        for (int i = 0; i < bufferSize; i++) {
            buffer[i] = 'A';
        }
        
        return new String(buffer);
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal size
        System.out.println("Test 1 (size 10): " + allocateAndFillBuffer(10));
        
        // Test case 2: Small size
        System.out.println("Test 2 (size 1): " + allocateAndFillBuffer(1));
        
        // Test case 3: Medium size
        System.out.println("Test 3 (size 50): " + allocateAndFillBuffer(50).substring(0, 20) + "...");
        
        // Test case 4: Larger size
        System.out.println("Test 4 (size 500): Length = " + allocateAndFillBuffer(500).length());
        
        // Test case 5: Edge case - try invalid size (should throw exception)
        try {
            allocateAndFillBuffer(0);
        } catch (IllegalArgumentException e) {
            System.out.println("Test 5: Caught expected exception - " + e.getMessage());
        }
    }
}
