import java.util.Arrays;

public class Lse59 {

    // A constant to prevent excessive memory allocation (e.g., to prevent OutOfMemoryError)
    private static final int MAX_BUFFER_SIZE = 1024 * 1024; // 1MB

    /**
     * Allocates a buffer of a given size and overwrites it with 'A'.
     * In Java, arrays are allocated on the heap, but the reference to them is on the stack.
     * This is the closest equivalent to the C/C++ stack allocation concept.
     * Java's array access is bounds-checked, preventing buffer overflows automatically.
     *
     * @param buffer_size The size of the buffer to allocate.
     */
    public static void fillBuffer(int buffer_size) {
        if (buffer_size < 0) {
            System.out.println("Error: Buffer size cannot be negative.");
            return;
        }
        if (buffer_size > MAX_BUFFER_SIZE) {
            System.out.println("Error: Requested buffer size " + buffer_size + " exceeds the maximum allowed size of " + MAX_BUFFER_SIZE + ".");
            return;
        }

        try {
            // The array 'buffer' is allocated on the heap.
            // The reference 'buffer' itself lives on the stack frame of this method.
            char[] buffer = new char[buffer_size];

            // Securely fill the buffer. It's not possible to write out of bounds.
            Arrays.fill(buffer, 'A');

            System.out.println("Successfully allocated and filled a buffer of size " + buffer_size + ".");
            // The buffer goes out of scope and is eligible for garbage collection here.

        } catch (OutOfMemoryError e) {
            System.err.println("Error: Ran out of memory trying to allocate buffer of size " + buffer_size + ".");
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");
        fillBuffer(16);          // Test Case 1: Small valid size
        fillBuffer(0);           // Test Case 2: Zero size
        fillBuffer(1024);        // Test Case 3: Larger valid size
        fillBuffer(-1);          // Test Case 4: Invalid negative size
        fillBuffer(MAX_BUFFER_SIZE + 1); // Test Case 5: Size exceeding the defined maximum
    }
}