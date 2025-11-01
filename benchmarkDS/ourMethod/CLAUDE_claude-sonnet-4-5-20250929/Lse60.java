
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse60 {
    // Maximum buffer size to prevent excessive memory allocation
    private static final int MAX_BUFFER_SIZE = 10485760; // 10 MB
    
    /**
     * Allocates and prints buffer contents
     * @param size The size of buffer to allocate
     * @return 0 on success, -1 on error
     */
    public static int allocateAndPrintBuffer(int size) {
        // Validate input: check for negative values and overflow
        if (size < 0) {
            System.err.println("Error: Buffer size cannot be negative");
            return -1;
        }
        
        // Enforce maximum size to prevent excessive memory usage
        if (size > MAX_BUFFER_SIZE) {
            System.err.println("Error: Buffer size exceeds maximum allowed");
            return -1;
        }
        
        try {
            // Allocate buffer - Java initializes arrays to zero by default
            byte[] buffer = new byte[size];
            
            // Print buffer contents (will be all zeros since newly allocated)
            // Using write instead of print for binary safety
            System.out.write(buffer, 0, buffer.length);
            System.out.flush();
            
            // Clear sensitive data before returning (defense in depth)
            Arrays.fill(buffer, (byte) 0);
            
            return 0;
        } catch (OutOfMemoryError e) {
            // Handle allocation failure without exposing internal details
            System.err.println("Error: Unable to allocate buffer");
            return -1;
        } catch (Exception e) {
            // Generic error handling
            System.err.println("Error: Operation failed");
            return -1;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        if (args.length == 0) {
            // Run test cases when no arguments provided
            System.out.println("Running test cases:");
            
            // Test 1: Normal small buffer
            System.out.println("\\nTest 1: Size 10");
            allocateAndPrintBuffer(10);
            
            // Test 2: Zero size buffer
            System.out.println("\\nTest 2: Size 0");
            allocateAndPrintBuffer(0);
            
            // Test 3: Moderate size buffer
            System.out.println("\\nTest 3: Size 1024");
            allocateAndPrintBuffer(1024);
            
            // Test 4: Negative size (should fail)
            System.out.println("\\nTest 4: Size -1");
            allocateAndPrintBuffer(-1);
            
            // Test 5: Excessive size (should fail)
            System.out.println("\\nTest 5: Size exceeding maximum");
            allocateAndPrintBuffer(MAX_BUFFER_SIZE + 1);
            
            return;
        }
        
        // Validate argument count
        if (args.length != 1) {
            System.err.println("Usage: java Lse60 <buffer_size>");
            System.exit(1);
        }
        
        try {
            // Parse and validate input with bounds checking
            int size = Integer.parseInt(args[0]);
            int result = allocateAndPrintBuffer(size);
            System.exit(result == 0 ? 0 : 1);
        } catch (NumberFormatException e) {
            // Handle invalid integer format without exposing details
            System.err.println("Error: Invalid buffer size");
            System.exit(1);
        }
    }
}
