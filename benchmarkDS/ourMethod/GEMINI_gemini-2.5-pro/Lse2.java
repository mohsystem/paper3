import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse2 {

    /**
     * Copies a number of bytes from a source byte array to a destination byte array.
     * This implementation includes boundary checks to prevent buffer overflows.
     *
     * @param dest The destination byte array.
     * @param src  The source byte array.
     * @param n    The number of bytes to copy.
     * @return true if the copy was successful, false otherwise.
     */
    public static boolean copyBytes(byte[] dest, byte[] src, int n) {
        // Rule #4: Input validation
        if (dest == null || src == null) {
            System.err.println("Error: Source or destination buffer is null.");
            return false;
        }
        if (n < 0) {
            System.err.println("Error: Number of bytes to copy cannot be negative.");
            return false;
        }
        if (n == 0) {
            return true; // Nothing to copy, operation is trivially successful
        }
        
        // Rules #1, #2: Ensure copy operations adhere to buffer boundaries.
        if (n > src.length) {
            System.err.println("Error: Attempting to read past the source buffer.");
            return false;
        }
        if (n > dest.length) {
            System.err.println("Error: Destination buffer is too small for the copy operation.");
            return false;
        }

        // Use the built-in, safe array copy method.
        System.arraycopy(src, 0, dest, 0, n);
        return true;
    }

    private static void printBuffer(String name, byte[] buffer, int len) {
        if (len < 0 || len > buffer.length) {
            len = buffer.length;
        }
        // Safely create a string from the relevant part of the buffer
        String content = new String(buffer, 0, len, StandardCharsets.UTF_8);
        System.out.println(name + " content: \"" + content + "\"");
    }

    public static void main(String[] args) {
        // Test Case 1: Normal successful copy
        System.out.println("--- Test Case 1: Normal Copy ---");
        byte[] src1 = "Hello, World!".getBytes(StandardCharsets.UTF_8);
        byte[] dest1 = new byte[50];
        int n1 = src1.length;
        System.out.println("Attempting to copy " + n1 + " bytes.");
        if (copyBytes(dest1, src1, n1)) {
            System.out.println("Copy successful.");
            printBuffer("Destination", dest1, n1);
        } else {
            System.out.println("Copy failed.");
        }
        System.out.println();

        // Test Case 2: Destination buffer too small
        System.out.println("--- Test Case 2: Destination Too Small ---");
        byte[] src2 = "This message is too long".getBytes(StandardCharsets.UTF_8);
        byte[] dest2 = new byte[10];
        int n2 = src2.length;
        System.out.println("Attempting to copy " + n2 + " bytes into a 10-byte buffer.");
        if (copyBytes(dest2, src2, n2)) {
            System.out.println("Copy successful.");
        } else {
            System.out.println("Copy failed as expected.");
        }
        System.out.println();

        // Test Case 3: Source read out of bounds
        System.out.println("--- Test Case 3: Source Read Out of Bounds ---");
        byte[] src3 = "short".getBytes(StandardCharsets.UTF_8);
        byte[] dest3 = new byte[20];
        int n3 = 10; // Try to copy 10 bytes from a 5-byte source
        System.out.println("Attempting to copy " + n3 + " bytes from a 5-byte source.");
        if (copyBytes(dest3, src3, n3)) {
            System.out.println("Copy successful.");
        } else {
            System.out.println("Copy failed as expected.");
        }
        System.out.println();
        
        // Test Case 4: Copy zero bytes
        System.out.println("--- Test Case 4: Zero Bytes Copy ---");
        byte[] src4 = "some data".getBytes(StandardCharsets.UTF_8);
        byte[] dest4 = new byte[20];
        byte[] originalDest4 = Arrays.copyOf(dest4, dest4.length);
        int n4 = 0;
        System.out.println("Attempting to copy " + n4 + " bytes.");
        if (copyBytes(dest4, src4, n4)) {
            System.out.println("Copy successful.");
            if (Arrays.equals(dest4, originalDest4)) {
                System.out.println("Destination buffer remains unchanged as expected.");
            } else {
                System.out.println("Error: Destination buffer was modified.");
            }
        } else {
            System.out.println("Copy failed.");
        }
        System.out.println();

        // Test Case 5: Exact size copy
        System.out.println("--- Test Case 5: Exact Size Copy ---");
        byte[] src5 = "exact fit".getBytes(StandardCharsets.UTF_8);
        byte[] dest5 = new byte[src5.length];
        int n5 = src5.length;
        System.out.println("Attempting to copy " + n5 + " bytes into a " + n5 + "-byte buffer.");
        if (copyBytes(dest5, src5, n5)) {
            System.out.println("Copy successful.");
            printBuffer("Destination", dest5, n5);
        } else {
            System.out.println("Copy failed.");
        }
        System.out.println();
    }
}