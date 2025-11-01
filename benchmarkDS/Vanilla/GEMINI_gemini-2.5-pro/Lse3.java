import java.util.Arrays;

public class Lse3 {

    /**
     * Copies a number of bytes from one byte array to another.
     *
     * @param dest The destination byte array.
     * @param src The source byte array.
     * @param n The number of bytes to copy.
     */
    public static void memoryCopy(byte[] dest, byte[] src, int n) {
        if (dest == null || src == null) {
            return;
        }
        // Ensure we don't go out of bounds
        int len = Math.min(Math.min(n, src.length), dest.length);
        for (int i = 0; i < len; i++) {
            dest[i] = src[i];
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Copy "Hello"
        System.out.println("--- Test Case 1: Copy 'Hello' ---");
        String str1 = "Hello";
        byte[] src1 = str1.getBytes();
        byte[] dest1 = new byte[10];
        memoryCopy(dest1, src1, src1.length);
        System.out.println("Source: " + str1);
        System.out.println("Destination: " + new String(dest1, 0, src1.length));
        System.out.println();

        // Test Case 2: Copy a longer string
        System.out.println("--- Test Case 2: Copy a longer string ---");
        String str2 = "This is a test string.";
        byte[] src2 = str2.getBytes();
        byte[] dest2 = new byte[30];
        memoryCopy(dest2, src2, src2.length);
        System.out.println("Source: " + str2);
        System.out.println("Destination: " + new String(dest2, 0, src2.length));
        System.out.println();

        // Test Case 3: Copy a portion of a string
        System.out.println("--- Test Case 3: Copy a portion ---");
        String str3 = "Programming";
        byte[] src3 = str3.getBytes();
        byte[] dest3 = new byte[15];
        memoryCopy(dest3, src3, 7); // Copy "Program"
        System.out.println("Source: " + str3);
        System.out.println("Destination (first 7 bytes): " + new String(dest3, 0, 7));
        System.out.println();
        
        // Test Case 4: Copy raw bytes (e.g., representing numbers)
        System.out.println("--- Test Case 4: Copy raw bytes ---");
        byte[] src4 = {10, 20, 30, 40, 50};
        byte[] dest4 = new byte[5];
        memoryCopy(dest4, src4, src4.length);
        System.out.println("Source bytes: " + Arrays.toString(src4));
        System.out.println("Destination bytes: " + Arrays.toString(dest4));
        System.out.println();

        // Test Case 5: Copy zero bytes
        System.out.println("--- Test Case 5: Copy zero bytes ---");
        String str5 = "Should not be copied";
        byte[] src5 = str5.getBytes();
        byte[] dest5 = new byte[20];
        Arrays.fill(dest5, (byte) 'X'); // Fill destination to see the effect
        memoryCopy(dest5, src5, 0);
        System.out.println("Source: " + str5);
        System.out.println("Destination after copying 0 bytes: " + new String(dest5));
        System.out.println();
    }
}