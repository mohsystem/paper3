import java.util.Arrays;

class Lse2 {
    /**
     * Copies a number of bytes from a source byte array to a destination byte array.
     *
     * @param dest The destination byte array.
     * @param src  The source byte array.
     * @param n    The number of bytes to copy.
     */
    public static void myMemcpy(byte[] dest, byte[] src, int n) {
        // 1. Security Check: Handle null pointers to prevent NullPointerException.
        if (dest == null || src == null) {
            System.err.println("Error: Destination or source array is null.");
            return;
        }

        // 2. Security Check: Ensure 'n' is not negative.
        if (n < 0) {
            System.err.println("Error: Number of bytes to copy cannot be negative.");
            return;
        }
        
        // 3. Security Check: Prevent buffer overflow by checking array bounds.
        // Ensure we do not read past the end of the source or write past the end of the destination.
        if (n > src.length || n > dest.length) {
            System.err.println("Error: Copying " + n + " bytes would cause a buffer overflow.");
            return;
        }

        // 4. Perform the copy operation.
        // System.arraycopy(src, 0, dest, 0, n); // Standard, optimized way
        // Manual implementation as requested by the prompt:
        for (int i = 0; i < n; i++) {
            dest[i] = src[i];
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---

        // Test Case 1: Standard copy of a string.
        System.out.println("--- Test Case 1: Standard copy ---");
        byte[] src1 = "Hello World".getBytes();
        byte[] dest1 = new byte[20];
        int n1 = 11;
        System.out.println("Before copy: dest1 = " + new String(dest1));
        myMemcpy(dest1, src1, n1);
        System.out.println("After copy:  dest1 = " + new String(dest1).trim());
        System.out.println();

        // Test Case 2: Partial copy of an integer array's byte representation.
        System.out.println("--- Test Case 2: Partial copy ---");
        byte[] src2 = {10, 20, 30, 40, 50};
        byte[] dest2 = new byte[10];
        int n2 = 3;
        System.out.println("Before copy: dest2 = " + Arrays.toString(dest2));
        myMemcpy(dest2, src2, n2);
        System.out.println("After copy:  dest2 = " + Arrays.toString(dest2));
        System.out.println();

        // Test Case 3: Zero-byte copy.
        System.out.println("--- Test Case 3: Zero-byte copy ---");
        byte[] src3 = "Source".getBytes();
        byte[] dest3 = new byte[10]; // Initialized to all zeros
        int n3 = 0;
        System.out.println("Before copy: dest3 = " + Arrays.toString(dest3));
        myMemcpy(dest3, src3, n3);
        System.out.println("After copy:  dest3 = " + Arrays.toString(dest3));
        System.out.println();

        // Test Case 4: Copying into a buffer that is smaller than the source, but copy size is safe.
        System.out.println("--- Test Case 4: Safe copy to smaller buffer ---");
        byte[] src4 = "This is a long source string".getBytes();
        byte[] dest4 = new byte[5];
        int n4 = 5;
        System.out.println("Before copy: dest4 = " + new String(dest4));
        myMemcpy(dest4, src4, n4);
        System.out.println("After copy:  dest4 = " + new String(dest4));
        System.out.println();

        // Test Case 5: Attempted buffer overflow (should be prevented).
        System.out.println("--- Test Case 5: Attempted buffer overflow ---");
        byte[] src5 = "Short".getBytes();
        byte[] dest5 = new byte[3];
        int n5 = 5; // n > dest.length
        System.out.println("Before copy: dest5 = " + Arrays.toString(dest5));
        myMemcpy(dest5, src5, n5); // This should print an error and not modify dest5
        System.out.println("After copy:  dest5 = " + Arrays.toString(dest5));
        System.out.println();
    }
}