import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse1 {

    // Copies 'n' bytes from src to dest. Returns number of bytes copied on success, -1 on error.
    public static int memCopy(byte[] dest, byte[] src, int n) {
        if (dest == null || src == null) {
            return -1;
        }
        if (n < 0) {
            return -1;
        }
        if (n > dest.length || n > src.length) {
            return -1;
        }
        if (n == 0) {
            return 0;
        }
        System.arraycopy(src, 0, dest, 0, n);
        return n;
    }

    private static String toHex(byte[] arr) {
        StringBuilder sb = new StringBuilder();
        for (byte b : arr) {
            sb.append(String.format("%02X", b)).append(' ');
        }
        return sb.toString().trim();
    }

    public static void main(String[] args) {
        // Test 1: normal copy of 5 bytes
        byte[] src1 = new byte[10];
        for (int i = 0; i < src1.length; i++) src1[i] = (byte) i;
        byte[] dest1 = new byte[10];
        int r1 = memCopy(dest1, src1, 5);
        System.out.println("Test1 result=" + r1 + " dest1=" + toHex(dest1));

        // Test 2: copy zero bytes
        byte[] src2 = new byte[] {1,2,3,4,5};
        byte[] dest2 = new byte[] {9,9,9,9,9};
        int r2 = memCopy(dest2, src2, 0);
        System.out.println("Test2 result=" + r2 + " dest2=" + toHex(dest2));

        // Test 3: copy full size exact
        byte[] src3 = "ABCDEFGH".getBytes(StandardCharsets.UTF_8);
        byte[] dest3 = new byte[8];
        int r3 = memCopy(dest3, src3, 8);
        System.out.println("Test3 result=" + r3 + " dest3=" + toHex(dest3));

        // Test 4: attempt to copy too many bytes (should fail)
        byte[] src4 = new byte[] {10,20,30,40,50};
        byte[] dest4 = new byte[] {0,0,0};
        int r4 = memCopy(dest4, src4, 4);
        System.out.println("Test4 result=" + r4 + " dest4=" + toHex(dest4));

        // Test 5: same array as both src and dest
        byte[] same = new byte[] {7,7,7,7,7,7};
        int r5 = memCopy(same, same, 6);
        System.out.println("Test5 result=" + r5 + " same=" + toHex(same));
    }
}