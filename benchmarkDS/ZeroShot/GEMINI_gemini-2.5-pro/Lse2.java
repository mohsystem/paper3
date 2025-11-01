public class Lse2 {

    /**
     * Securely copies a specified number of bytes from a source byte array to a destination byte array.
     * The function prevents null pointer access, negative copy sizes, and buffer overflows.
     *
     * @param dest     The destination byte array.
     * @param src      The source byte array.
     * @param numBytes The number of bytes to copy.
     * @return true if the copy was successful, false otherwise.
     */
    public static boolean copyBytes(byte[] dest, byte[] src, int numBytes) {
        // 1. Check for null pointers
        if (dest == null || src == null) {
            System.err.println("Error: Source or destination array is null.");
            return false;
        }

        // 2. Check for invalid size
        if (numBytes < 0) {
            System.err.println("Error: Number of bytes to copy cannot be negative.");
            return false;
        }
        
        // 3. Check for buffer overflow on destination
        if (numBytes > dest.length) {
            System.err.println("Error: Copy would result in a buffer overflow on the destination.");
            return false;
        }

        // 4. Check for buffer over-read on source
        if (numBytes > src.length) {
            System.err.println("Error: Attempting to read past the end of the source buffer.");
            return false;
        }

        // Use the built-in, bounds-checked array copy method which is inherently safe
        System.arraycopy(src, 0, dest, 0, numBytes);
        return true;
    }

    public static void main(String[] args) {
        // Test Case 1: Normal successful copy
        System.out.println("--- Test Case 1: Normal Copy ---");
        byte[] src1 = "Hello World".getBytes();
        byte[] dest1 = new byte[20];
        boolean result1 = copyBytes(dest1, src1, 11);
        System.out.println("Result: " + (result1 ? "Success" : "Failure"));
        if(result1) System.out.println("Destination: \"" + new String(dest1, 0, 11) + "\"");
        System.out.println();

        // Test Case 2: Boundary case (full buffer)
        System.out.println("--- Test Case 2: Full Buffer Copy ---");
        byte[] src2 = "Test".getBytes();
        byte[] dest2 = new byte[4];
        boolean result2 = copyBytes(dest2, src2, 4);
        System.out.println("Result: " + (result2 ? "Success" : "Failure"));
        if(result2) System.out.println("Destination: \"" + new String(dest2) + "\"");
        System.out.println();

        // Test Case 3: Destination buffer overflow attempt
        System.out.println("--- Test Case 3: Destination Overflow Attempt ---");
        byte[] src3 = "This string is too long for the destination".getBytes();
        byte[] dest3 = new byte[5];
        java.util.Arrays.fill(dest3, (byte)'X'); // Pre-fill to see if it's changed
        boolean result3 = copyBytes(dest3, src3, src3.length);
        System.out.println("Result: " + (result3 ? "Success" : "Failure"));
        System.out.println("Destination (should be unchanged): \"" + new String(dest3) + "\"");
        System.out.println();

        // Test Case 4: Source buffer over-read attempt
        System.out.println("--- Test Case 4: Source Over-read Attempt ---");
        byte[] src4 = "short".getBytes();
        byte[] dest4 = new byte[20];
        boolean result4 = copyBytes(dest4, src4, 10);
        System.out.println("Result: " + (result4 ? "Success" : "Failure"));
        System.out.println("Destination (should be unchanged): \"" + new String(dest4).trim() + "\"");
        System.out.println();

        // Test Case 5: Null pointer input
        System.out.println("--- Test Case 5: Null Input ---");
        byte[] src5 = "test".getBytes();
        byte[] dest5 = null;
        boolean result5 = copyBytes(dest5, src5, 4);
        System.out.println("Result (null destination): " + (result5 ? "Success" : "Failure"));
        result5 = copyBytes(new byte[10], null, 4);
        System.out.println("Result (null source): " + (result5 ? "Success" : "Failure"));
    }
}