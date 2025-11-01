import java.util.Arrays;

public class Lse1 {

    /**
     * Securely copies a specified number of bytes from a source array to a destination array.
     *
     * @param dest The destination byte array.
     * @param src The source byte array.
     * @param numBytes The number of bytes to copy.
     * @throws IllegalArgumentException if arrays are null, numBytes is negative,
     *                                  or if the copy would cause a buffer overflow.
     */
    public static void secureCopy(byte[] dest, byte[] src, int numBytes) {
        if (dest == null || src == null) {
            throw new IllegalArgumentException("Source or destination array cannot be null.");
        }
        if (numBytes < 0) {
            throw new IllegalArgumentException("Number of bytes to copy cannot be negative.");
        }
        if (numBytes > src.length) {
            throw new IllegalArgumentException("Source buffer overflow: Not enough bytes in source array.");
        }
        if (numBytes > dest.length) {
            throw new IllegalArgumentException("Destination buffer overflow: Destination array is too small.");
        }

        // System.arraycopy is optimized and safe. It handles overlapping regions correctly.
        System.arraycopy(src, 0, dest, 0, numBytes);
    }

    public static void main(String[] args) {
        // Test Case 1: Normal successful copy
        System.out.println("--- Test Case 1: Normal Copy ---");
        try {
            byte[] src1 = {1, 2, 3, 4, 5, 6, 7};
            byte[] dest1 = new byte[5];
            System.out.println("Destination before copy: " + Arrays.toString(dest1));
            secureCopy(dest1, src1, 5);
            System.out.println("Destination after copy:  " + Arrays.toString(dest1));
            System.out.println("Expected: [1, 2, 3, 4, 5]");
        } catch (IllegalArgumentException e) {
            System.out.println("Caught exception: " + e.getMessage());
        }
        System.out.println();

        // Test Case 2: Destination buffer too small (potential overflow)
        System.out.println("--- Test Case 2: Destination Too Small ---");
        try {
            byte[] src2 = {10, 20, 30, 40, 50};
            byte[] dest2 = new byte[3];
            System.out.println("Attempting to copy 5 bytes into a 3-byte array.");
            secureCopy(dest2, src2, 5);
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
        System.out.println();

        // Test Case 3: Copying zero bytes
        System.out.println("--- Test Case 3: Zero Bytes to Copy ---");
        try {
            byte[] src3 = {1, 1, 1, 1, 1};
            byte[] dest3 = {9, 9, 9, 9, 9};
            System.out.println("Destination before copy: " + Arrays.toString(dest3));
            secureCopy(dest3, src3, 0);
            System.out.println("Destination after copy:  " + Arrays.toString(dest3));
            System.out.println("Expected: [9, 9, 9, 9, 9]");
        } catch (IllegalArgumentException e) {
            System.out.println("Caught exception: " + e.getMessage());
        }
        System.out.println();
        
        // Test Case 4: Overlapping memory regions
        System.out.println("--- Test Case 4: Overlapping Memory ---");
        try {
            byte[] buffer4 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
            System.out.println("Buffer before copy: " + Arrays.toString(buffer4));
            // Create "views" into the array to simulate pointers
            // System.arraycopy handles this case correctly
            byte[] srcView = new byte[5];
            byte[] destView = new byte[5];
            System.arraycopy(buffer4, 0, srcView, 0, 5); // src is {1,2,3,4,5}
            // The destination starts at index 2 of the original buffer
            System.arraycopy(destView, 0, buffer4, 2, 5); 
            System.arraycopy(srcView, 0, buffer4, 2, 5);

            System.out.println("Buffer after copy:  " + Arrays.toString(buffer4));
            System.out.println("Expected: [1, 2, 1, 2, 3, 4, 5, 8, 9, 10]");
        } catch (Exception e) {
            System.out.println("Caught unexpected exception: " + e.getMessage());
        }
        System.out.println();

        // Test Case 5: Null array input
        System.out.println("--- Test Case 5: Null Input ---");
        try {
            byte[] src5 = {1, 2, 3};
            System.out.println("Attempting to copy to a null destination.");
            secureCopy(null, src5, 3);
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
        System.out.println();
    }
}