import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse1 {
    public static byte[] memmoveBytes(byte[] dest, byte[] src, int count) {
        if (dest == null || src == null) {
            throw new IllegalArgumentException("Source and destination must not be null");
        }
        if (count < 0) {
            throw new IllegalArgumentException("Count must be non-negative");
        }
        if (count > dest.length || count > src.length) {
            throw new IllegalArgumentException("Count exceeds buffer length");
        }
        if (count == 0 || dest == src) {
            return dest;
        }
        // System.arraycopy handles overlap when dest and src are the same array.
        System.arraycopy(src, 0, dest, 0, count);
        return dest;
    }

    private static String toHex(byte[] buf) {
        StringBuilder sb = new StringBuilder();
        for (byte b : buf) {
            sb.append(String.format("%02X", b)).append(" ");
        }
        return sb.toString().trim();
    }

    public static void main(String[] args) {
        // Test 1: Copy 10 bytes "HelloWorld"
        byte[] dest1 = new byte[10];
        byte[] src1 = "HelloWorld".getBytes(StandardCharsets.UTF_8);
        memmoveBytes(dest1, src1, 10);
        System.out.println("Test1: " + new String(dest1, StandardCharsets.UTF_8));

        // Test 2: Copy 3 bytes from "abcdef"
        byte[] dest2 = new byte[6];
        byte[] src2 = "abcdef".getBytes(StandardCharsets.UTF_8);
        memmoveBytes(dest2, src2, 3);
        System.out.println("Test2: " + new String(dest2, StandardCharsets.UTF_8) + " | Hex: " + toHex(dest2));

        // Test 3: Zero bytes copy (no change)
        byte[] dest3 = new byte[3];
        byte[] src3 = "xyz".getBytes(StandardCharsets.UTF_8);
        memmoveBytes(dest3, src3, 0);
        System.out.println("Test3: " + Arrays.toString(dest3) + " | Hex: " + toHex(dest3));

        // Test 4: Invalid count (exceeds dest)
        try {
            byte[] dest4 = new byte[2];
            byte[] src4 = "12345".getBytes(StandardCharsets.UTF_8);
            memmoveBytes(dest4, src4, 5);
            System.out.println("Test4: Should not reach here");
        } catch (IllegalArgumentException ex) {
            System.out.println("Test4: Caught exception - " + ex.getMessage());
        }

        // Test 5: Same array as both src and dest
        byte[] both = "12345".getBytes(StandardCharsets.UTF_8);
        memmoveBytes(both, both, 4);
        System.out.println("Test5: " + new String(both, StandardCharsets.UTF_8));
    }
}