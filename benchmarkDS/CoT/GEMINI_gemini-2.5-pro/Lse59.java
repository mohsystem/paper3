import java.util.Arrays;

public class Lse59 {

    // In Java, objects and arrays are allocated on the heap.
    // The reference to the array/object is stored on the stack for local variables.
    // This function simulates the requested behavior in an idiomatic Java way.
    // A hard limit is added to prevent excessively large allocations that could lead to OutOfMemoryError.
    private static final int MAX_BUFFER_SIZE = 10 * 1024 * 1024; // 10 MB limit for safety

    /**
     * Allocates a character array (on the heap) of a given size and fills it with 'A'.
     *
     * @param bufferSize The size of the buffer to allocate.
     */
    public static void allocateAndFillBuffer(int bufferSize) {
        if (bufferSize < 0) {
            System.out.println("Error: Buffer size cannot be negative.");
            return;
        }
        if (bufferSize > MAX_BUFFER_SIZE) {
            System.out.println("Error: Requested buffer size " + bufferSize + " exceeds the maximum allowed size of " + MAX_BUFFER_SIZE + ".");
            return;
        }

        try {
            // The array 'buffer' is allocated on the heap.
            // The reference to it, 'buffer', is on the stack.
            char[] buffer = new char[bufferSize];

            // Overwrite all values in the buffer with 'A'.
            Arrays.fill(buffer, 'A');

            System.out.println("Successfully allocated and filled a buffer of size " + bufferSize + " with 'A'.");
            // The buffer goes out of scope and is eligible for garbage collection here.
            
        } catch (OutOfMemoryError e) {
            System.out.println("Error: Ran out of memory trying to allocate buffer of size " + bufferSize + ".");
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");
        // Test Case 1: A small, typical size
        allocateAndFillBuffer(16);
        // Test Case 2: Size 1
        allocateAndFillBuffer(1);
        // Test Case 3: Size 0 (edge case)
        allocateAndFillBuffer(0);
        // Test Case 4: A larger, but valid size
        allocateAndFillBuffer(1024);
        // Test Case 5: A size that is too large and will be caught by our security check
        allocateAndFillBuffer(20_000_000);
    }
}