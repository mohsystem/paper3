import java.util.Arrays;

public class Lse1 {

    /**
     * Copies a number of bytes from a source byte array to a destination byte array.
     * This implementation performs bounds checking to prevent buffer overflows.
     *
     * @param dest     The destination byte array.
     * @param src      The source byte array.
     * @param numBytes The number of bytes to copy from the beginning of src to the beginning of dest.
     * @return true if the copy was successful, false otherwise.
     */
    public static boolean copyBytes(byte[] dest, byte[] src, int numBytes) {
        // Rule #4: Ensure all input is validated.
        // Check for null references.
        if (dest == null || src == null) {
            System.err.println("Error: Source or destination array is null.");
            return false;
        }

        // Check for invalid number of bytes.
        if (numBytes < 0) {
            System.err.println("Error: Number of bytes to copy cannot be negative.");
            return false;
        }

        // Rule #1 & #2: Ensure that buffer sizes are checked before performing any copy operations.
        // Check if the copy operation will go out of bounds for either array.
        if (numBytes > src.length) {
            System.err.println("Error: Cannot copy " + numBytes + " bytes from a source of size " + src.length + ".");
            return false;
        }
        if (numBytes > dest.length) {
            System.err.println("Error: Cannot copy " + numBytes + " bytes to a destination of size " + dest.length + ".");
            return false;
        }

        // Rule #5: Use safer alternatives where possible. System.arraycopy is bounds-checked.
        System.arraycopy(src, 0, dest, 0, numBytes);
        return true;
    }

    public static void main(String[] args) {
        // Test Case 1: Normal successful copy
        System.out.println("--- Test Case 1: Normal Copy ---");
        byte[] src1 = {10, 20, 30, 40, 50};
        byte[] dest1 = new byte[5];
        if (copyBytes(dest1, src1, 5)) {
            System.out.println("Source:      " + Arrays.toString(src1));
            System.out.println("Destination: " + Arrays.toString(dest1));
            System.out.println("Result: Success");
        } else {
            System.out.println("Result: Failed");
        }
        System.out.println();

        // Test Case 2: Attempt to copy more bytes than source contains
        System.out.println("--- Test Case 2: Source Overflow ---");
        byte[] src2 = {1, 2, 3};
        byte[] dest2 = new byte[5];
        if (copyBytes(dest2, src2, 5)) {
             System.out.println("Result: Success");
        } else {
             System.out.println("Result: Failed as expected");
        }
        System.out.println();

        // Test Case 3: Attempt to copy more bytes than destination can hold
        System.out.println("--- Test Case 3: Destination Overflow ---");
        byte[] src3 = {1, 2, 3, 4, 5};
        byte[] dest3 = new byte[3];
        if (copyBytes(dest3, src3, 5)) {
             System.out.println("Result: Success");
        } else {
             System.out.println("Result: Failed as expected");
        }
        System.out.println();
        
        // Test Case 4: Copy zero bytes
        System.out.println("--- Test Case 4: Zero Bytes Copy ---");
        byte[] src4 = {1, 2, 3, 4, 5};
        byte[] dest4 = new byte[5]; // Initialized to all zeros
        if (copyBytes(dest4, src4, 0)) {
            System.out.println("Source:      " + Arrays.toString(src4));
            System.out.println("Destination: " + Arrays.toString(dest4));
            System.out.println("Result: Success (destination is unchanged)");
        } else {
             System.out.println("Result: Failed");
        }
        System.out.println();

        // Test Case 5: Null input
        System.out.println("--- Test Case 5: Null Input ---");
        byte[] src5 = {1, 2, 3};
        if (copyBytes(null, src5, 3)) {
             System.out.println("Result: Success");
        } else {
             System.out.println("Result: Failed as expected");
        }
        System.out.println();
    }
}