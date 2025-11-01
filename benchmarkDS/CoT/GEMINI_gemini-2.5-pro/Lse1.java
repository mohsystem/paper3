import java.util.Arrays;

public class Lse1 {

    /**
     * Copies a number of bytes from a source byte array to a destination byte array.
     *
     * @param destination The destination byte array.
     * @param source      The source byte array.
     * @param numBytes    The number of bytes to copy from the beginning of the source to the beginning of the destination.
     */
    public static void memoryCopy(byte[] destination, byte[] source, int numBytes) {
        // 1. Security Check: Validate inputs to prevent NullPointerException and invalid arguments.
        if (destination == null || source == null) {
            // Silently return to prevent crashing on null inputs.
            return;
        }
        
        // 2. Security Check: If numBytes is non-positive, there's nothing to copy.
        if (numBytes <= 0) {
            return;
        }

        // 3. Security Check: Prevent Buffer Overflow by checking array bounds.
        if (numBytes > source.length || numBytes > destination.length) {
            // Cannot copy more bytes than what is available in either array.
            return;
        }
        
        // 4. Core Logic: Perform the byte-by-byte copy.
        // This loop is safe due to the preceding bounds checks.
        for (int i = 0; i < numBytes; i++) {
            destination[i] = source[i];
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Normal copy
        byte[] src1 = "Hello World".getBytes();
        byte[] dest1 = new byte[11];
        memoryCopy(dest1, src1, 11);
        System.out.println("Test 1 (Normal): " + new String(dest1)); // Expected: Hello World

        // Test Case 2: Partial copy
        byte[] src2 = "Programming".getBytes();
        byte[] dest2 = new byte[11];
        memoryCopy(dest2, src2, 7);
        System.out.println("Test 2 (Partial): " + new String(dest2, 0, 7)); // Expected: Program

        // Test Case 3: Zero-byte copy
        byte[] src3 = "Test".getBytes();
        byte[] dest3 = new byte[4];
        byte[] originalDest3 = Arrays.copyOf(dest3, dest3.length);
        memoryCopy(dest3, src3, 0);
        System.out.println("Test 3 (Zero bytes): " + (Arrays.equals(dest3, originalDest3) ? "Unchanged" : "Changed")); // Expected: Unchanged

        // Test Case 4: Null input (should not crash)
        byte[] src4 = "Test".getBytes();
        try {
            memoryCopy(null, src4, 4);
            memoryCopy(src4, null, 4);
            System.out.println("Test 4 (Null input): Handled gracefully");
        } catch (Exception e) {
            System.out.println("Test 4 (Null input): Crashed - " + e.getMessage());
        }

        // Test Case 5: Out-of-bounds copy (should not perform the copy)
        byte[] src5 = "Small".getBytes();
        byte[] dest5 = new byte[3];
        byte[] originalDest5 = Arrays.copyOf(dest5, dest5.length);
        memoryCopy(dest5, src5, 5); // Attempt to copy 5 bytes into a 3-byte array
        System.out.println("Test 5 (Out of bounds): " + (Arrays.equals(dest5, originalDest5) ? "Handled correctly" : "Failed")); // Expected: Handled correctly
    }
}