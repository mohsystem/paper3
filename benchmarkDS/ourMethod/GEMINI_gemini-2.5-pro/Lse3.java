import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse3 {

    /**
     * Copies a specified number of bytes from a source byte array to a destination byte array.
     * This implementation manually checks boundaries before delegating to the optimized 
     * and safe System.arraycopy method.
     *
     * @param dest The destination byte array. Must not be null.
     * @param src  The source byte array. Must not be null.
     * @param n    The number of bytes to copy. Must not be negative and must be within the
     *             bounds of both source and destination arrays.
     */
    public static void customMemcpy(byte[] dest, byte[] src, int n) {
        // Rule #4: Input validation
        if (dest == null || src == null) {
            System.err.println("Error: Source or destination array is null.");
            return;
        }

        // Rule #1, #2: Ensure that buffer sizes are checked before performing any copy operations.
        if (n < 0) {
            System.err.println("Error: Number of bytes to copy cannot be negative.");
            return;
        }
        // Check read boundary on source
        if (src.length < n) {
            System.err.println("Error: Source array is smaller than the number of bytes to copy.");
            return;
        }
        // Check write boundary on destination
        if (dest.length < n) {
            System.err.println("Error: Destination array is not large enough.");
            return;
        }

        // Use the safe and optimized built-in method for the actual copy.
        // It performs its own bounds checking, but our explicit checks provide clearer error messages.
        System.arraycopy(src, 0, dest, 0, n);
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Copy "Hello"
        System.out.println("\n--- Test Case 1: Copy 'Hello' ---");
        String helloStr = "Hello";
        byte[] src1 = helloStr.getBytes(StandardCharsets.UTF_8);
        byte[] dest1 = new byte[10];
        int bytesToCopy1 = src1.length;
        
        System.out.println("Source: " + new String(src1, StandardCharsets.UTF_8));
        System.out.println("Destination (before): " + Arrays.toString(dest1));
        customMemcpy(dest1, src1, bytesToCopy1);
        System.out.println("Destination (after):  " + Arrays.toString(dest1));
        System.out.println("Copied part as string: \"" + new String(dest1, 0, bytesToCopy1, StandardCharsets.UTF_8) + "\"");

        // Test Case 2: Copy an empty source (0 bytes)
        System.out.println("\n--- Test Case 2: Copy an empty source (0 bytes) ---");
        byte[] src2 = "".getBytes(StandardCharsets.UTF_8);
        byte[] dest2 = new byte[10];
        System.out.println("Destination (before): " + Arrays.toString(dest2));
        customMemcpy(dest2, src2, 0);
        System.out.println("Destination (after):  " + Arrays.toString(dest2));

        // Test Case 3: Copy part of a longer string
        System.out.println("\n--- Test Case 3: Copy part of a string ---");
        byte[] src3 = "World12345".getBytes(StandardCharsets.UTF_8);
        byte[] dest3 = new byte[5];
        System.out.println("Source: " + new String(src3, StandardCharsets.UTF_8));
        customMemcpy(dest3, src3, 5); // Copy "World"
        System.out.println("Copied part as string: \"" + new String(dest3, StandardCharsets.UTF_8) + "\"");

        // Test Case 4: Destination buffer too small (error case)
        System.out.println("\n--- Test Case 4: Destination buffer too small ---");
        byte[] src4 = "This is too long".getBytes(StandardCharsets.UTF_8);
        byte[] dest4 = new byte[5];
        customMemcpy(dest4, src4, src4.length); // Should print an error and not crash

        // Test Case 5: Copy from one buffer to another
        System.out.println("\n--- Test Case 5: Copy from buffer to buffer ---");
        byte[] src5 = {10, 20, 30, 40, 50};
        byte[] dest5 = new byte[5];
        System.out.println("Source: " + Arrays.toString(src5));
        System.out.println("Destination (before): " + Arrays.toString(dest5));
        customMemcpy(dest5, src5, src5.length);
        System.out.println("Destination (after):  " + Arrays.toString(dest5));
        System.out.println("Arrays are equal: " + Arrays.equals(src5, dest5));
    }
}