import java.util.Arrays;

public class Lse59 {

    private static final int MAX_BUFFER_SIZE = 4096;

    /**
     * Allocates a local buffer of a given size and overwrites it with 'A'.
     * The buffer size is validated to be within a safe limit to prevent
     * excessive memory allocation. In Java, arrays are heap-allocated,
     * but a local reference makes this the idiomatic equivalent of a
     * stack-based buffer.
     *
     * @param bufferSize The size of the buffer to allocate.
     */
    public static void overwriteBuffer(int bufferSize) {
        if (bufferSize <= 0 || bufferSize > MAX_BUFFER_SIZE) {
            System.err.println("Error: Invalid buffer size. Must be between 1 and " + MAX_BUFFER_SIZE + ".");
            return;
        }

        char[] buffer = new char[bufferSize];
        
        Arrays.fill(buffer, 'A');

        System.out.println("Successfully allocated and overwrote a buffer of size " + bufferSize + ".");
        // Basic check to confirm the operation
        if (buffer.length > 0 && (buffer[0] != 'A' || buffer[buffer.length - 1] != 'A')) {
             System.err.println("Buffer content verification failed.");
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Test Case 1: Valid size (16) ---");
        overwriteBuffer(16);

        System.out.println("\n--- Test Case 2: Edge case size (1) ---");
        overwriteBuffer(1);
        
        System.out.println("\n--- Test Case 3: Maximum valid size (" + MAX_BUFFER_SIZE + ") ---");
        overwriteBuffer(MAX_BUFFER_SIZE);

        System.out.println("\n--- Test Case 4: Invalid size (0) ---");
        overwriteBuffer(0);

        System.out.println("\n--- Test Case 5: Invalid size (too large) ---");
        overwriteBuffer(MAX_BUFFER_SIZE + 1);
    }
}