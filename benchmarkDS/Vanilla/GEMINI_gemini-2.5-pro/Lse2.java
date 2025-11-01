import java.util.Arrays;

public class Lse2 {

    /**
     * Copies a number of bytes from one byte array to another.
     *
     * @param destination The destination byte array.
     * @param source      The source byte array.
     * @param numBytes    The number of bytes to copy.
     */
    public static void memoryCopy(byte[] destination, byte[] source, int numBytes) {
        if (destination == null || source == null) {
            return;
        }
        // Ensure we don't copy more bytes than are available in source or can fit in destination
        int length = Math.min(numBytes, Math.min(source.length, destination.length));
        System.arraycopy(source, 0, destination, 0, length);
    }

    public static void main(String[] args) {
        // Test Case 1: Basic copy
        System.out.println("--- Test Case 1: Basic Copy ---");
        byte[] src1 = "Hello World".getBytes();
        byte[] dest1 = new byte[15];
        System.out.println("Destination before copy: " + new String(dest1));
        memoryCopy(dest1, src1, 5);
        System.out.println("Destination after copy:  " + new String(dest1).trim());
        System.out.println();

        // Test Case 2: Partial copy
        System.out.println("--- Test Case 2: Partial Copy ---");
        byte[] src2 = "Programming".getBytes();
        byte[] dest2 = new byte[15];
        System.out.println("Destination before copy: " + new String(dest2));
        memoryCopy(dest2, src2, 4);
        System.out.println("Destination after copy:  " + new String(dest2).trim());
        System.out.println();

        // Test Case 3: Overwriting existing data
        System.out.println("--- Test Case 3: Overwriting Existing Data ---");
        byte[] src3 = "NewData".getBytes();
        byte[] dest3 = "OldDataOldData".getBytes();
        System.out.println("Destination before copy: " + new String(dest3));
        memoryCopy(dest3, src3, 7);
        System.out.println("Destination after copy:  " + new String(dest3));
        System.out.println();

        // Test Case 4: Copying zero bytes
        System.out.println("--- Test Case 4: Copying Zero Bytes ---");
        byte[] src4 = "Source".getBytes();
        byte[] dest4 = "Destination".getBytes();
        String originalDest = new String(dest4);
        System.out.println("Destination before copy: " + originalDest);
        memoryCopy(dest4, src4, 0);
        System.out.println("Destination after copy:  " + new String(dest4));
        System.out.println("Unchanged: " + originalDest.equals(new String(dest4)));
        System.out.println();

        // Test Case 5: Copying more bytes than source length
        System.out.println("--- Test Case 5: Copying More Bytes Than Source ---");
        byte[] src5 = "Short".getBytes();
        byte[] dest5 = new byte[10];
        System.out.println("Destination before copy (as bytes): " + Arrays.toString(dest5));
        memoryCopy(dest5, src5, 10); // Should only copy 5 bytes
        System.out.println("Destination after copy (as string): " + new String(dest5).trim());
        System.out.println("Destination after copy (as bytes):  " + Arrays.toString(dest5));
        System.out.println();
    }
}