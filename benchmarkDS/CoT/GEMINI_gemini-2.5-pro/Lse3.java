public class Lse3 {

    /**
     * Copies a specified number of bytes from a source byte array to a destination byte array.
     *
     * @param dest The destination byte array.
     * @param src  The source byte array.
     * @param n    The number of bytes to copy.
     */
    public static void myMemcpy(byte[] dest, byte[] src, int n) {
        // Security Check: Ensure arrays are not null.
        if (dest == null || src == null) {
            System.err.println("Error: Source or destination array is null.");
            return;
        }
        // Security Check: Ensure n is not negative.
        if (n < 0) {
            System.err.println("Error: Number of bytes cannot be negative.");
            return;
        }
        // Security Check: Prevent buffer overflow by checking array boundaries.
        if (n > src.length || n > dest.length) {
            System.err.println("Error: Buffer overflow attempt detected. Copy operation aborted.");
            return;
        }

        // Perform the byte-by-byte copy.
        for (int i = 0; i < n; i++) {
            dest[i] = src[i];
        }
    }

    public static void main(String[] args) {
        System.out.println("Java Tests:");

        // Test Case 1: Standard copy "Hello"
        byte[] src1 = "Hello".getBytes();
        byte[] dest1 = new byte[10];
        System.out.println("\n--- Test 1: Standard copy ---");
        System.out.println("Copying " + src1.length + " bytes from 'Hello'.");
        System.out.println("Destination buffer size: " + dest1.length);
        myMemcpy(dest1, src1, src1.length);
        System.out.println("Destination after copy: " + new String(dest1, 0, src1.length));

        // Test Case 2: Full buffer copy
        byte[] src2 = "12345".getBytes();
        byte[] dest2 = new byte[5];
        System.out.println("\n--- Test 2: Full buffer copy ---");
        System.out.println("Copying " + src2.length + " bytes from '12345'.");
        System.out.println("Destination buffer size: " + dest2.length);
        myMemcpy(dest2, src2, src2.length);
        System.out.println("Destination after copy: " + new String(dest2));

        // Test Case 3: Partial copy
        byte[] src3 = "World".getBytes();
        byte[] dest3 = new byte[10];
        System.out.println("\n--- Test 3: Partial copy ---");
        System.out.println("Copying 3 bytes from 'World'.");
        myMemcpy(dest3, src3, 3);
        System.out.println("Destination after copy: " + new String(dest3, 0, 3));

        // Test Case 4: Copy empty source
        byte[] src4 = "".getBytes();
        byte[] dest4 = new byte[5];
        java.util.Arrays.fill(dest4, (byte)'X');
        System.out.println("\n--- Test 4: Copy empty source ---");
        System.out.println("Destination before: " + new String(dest4));
        myMemcpy(dest4, src4, 0); // Copy 0 bytes
        System.out.println("Destination after copy (should be unchanged): " + new String(dest4));

        // Test Case 5: Array to array copy
        byte[] src5 = new byte[] {'A', 'B', 'C', 'D', 'E'};
        byte[] dest5 = new byte[5];
        System.out.println("\n--- Test 5: Array to array copy ---");
        myMemcpy(dest5, src5, src5.length);
        System.out.println("Destination after copy: " + new String(dest5));
    }
}