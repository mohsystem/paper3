import java.util.Arrays;

public class Lse1 {

    /**
     * Copies n bytes from the source array to the destination array,
     * starting from index 0 in both arrays.
     *
     * @param destination The destination byte array.
     * @param source      The source byte array.
     * @param n           The number of bytes to copy.
     */
    public static void copyBytes(byte[] destination, byte[] source, int n) {
        if (destination == null || source == null) {
            return;
        }
        // Ensure we don't go out of bounds
        int length = Math.min(n, Math.min(destination.length, source.length));
        for (int i = 0; i < length; i++) {
            destination[i] = source[i];
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Basic full copy
        System.out.println("--- Test Case 1: Basic full copy ---");
        byte[] src1 = {1, 2, 3, 4, 5};
        byte[] dest1 = new byte[5];
        System.out.println("Before: dest1 = " + Arrays.toString(dest1));
        copyBytes(dest1, src1, 5);
        System.out.println("After:  dest1 = " + Arrays.toString(dest1));
        System.out.println();

        // Test Case 2: Partial copy
        System.out.println("--- Test Case 2: Partial copy ---");
        byte[] src2 = {10, 20, 30, 40, 50};
        byte[] dest2 = {99, 98, 97, 96, 95};
        System.out.println("Before: dest2 = " + Arrays.toString(dest2));
        copyBytes(dest2, src2, 3);
        System.out.println("After:  dest2 = " + Arrays.toString(dest2));
        System.out.println();

        // Test Case 3: Zero-length copy
        System.out.println("--- Test Case 3: Zero-length copy ---");
        byte[] src3 = {1, 1, 1};
        byte[] dest3 = {2, 2, 2};
        System.out.println("Before: dest3 = " + Arrays.toString(dest3));
        copyBytes(dest3, src3, 0);
        System.out.println("After:  dest3 = " + Arrays.toString(dest3));
        System.out.println();

        // Test Case 4: Full overwrite of a smaller source into a larger destination
        System.out.println("--- Test Case 4: Overwriting a larger destination ---");
        byte[] src4 = {7, 8};
        byte[] dest4 = {1, 2, 3, 4};
        System.out.println("Before: dest4 = " + Arrays.toString(dest4));
        copyBytes(dest4, src4, 2);
        System.out.println("After:  dest4 = " + Arrays.toString(dest4));
        System.out.println();

        // Test Case 5: Copying into a larger buffer
        System.out.println("--- Test Case 5: Copying into a larger buffer ---");
        byte[] src5 = {1, 2, 3};
        byte[] dest5 = new byte[6];
        System.out.println("Before: dest5 = " + Arrays.toString(dest5));
        copyBytes(dest5, src5, 3);
        System.out.println("After:  dest5 = " + Arrays.toString(dest5));
        System.out.println();
    }
}