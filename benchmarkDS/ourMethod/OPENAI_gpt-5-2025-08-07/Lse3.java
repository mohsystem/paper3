import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse3 {
    public static int copyBytes(byte[] dest, byte[] src, int n) {
        if (dest == null || src == null) {
            throw new IllegalArgumentException("Destination and source must not be null");
        }
        if (n < 0) {
            throw new IllegalArgumentException("Number of bytes to copy must be non-negative");
        }
        int max = Math.min(n, Math.min(dest.length, src.length));
        if (max > 0) {
            System.arraycopy(src, 0, dest, 0, max);
        }
        return max;
    }

    public static void main(String[] args) {
        byte[] hello = "Hello".getBytes(StandardCharsets.UTF_8);

        // Test 1: Exact fit
        byte[] dest1 = new byte[5];
        int copied1 = copyBytes(dest1, hello, 5);
        System.out.println("Test1 copied: " + copied1 + " data: " + new String(dest1, 0, copied1, StandardCharsets.UTF_8));

        // Test 2: Destination smaller than requested
        byte[] dest2 = new byte[3];
        int copied2 = copyBytes(dest2, hello, 5);
        System.out.println("Test2 copied: " + copied2 + " data: " + new String(dest2, 0, copied2, StandardCharsets.UTF_8));

        // Test 3: Destination larger than requested (exact request)
        byte[] dest3 = new byte[10];
        int copied3 = copyBytes(dest3, hello, 5);
        System.out.println("Test3 copied: " + copied3 + " data: " + new String(dest3, 0, copied3, StandardCharsets.UTF_8));

        // Test 4: Copy fewer bytes than available
        byte[] dest4 = new byte[10];
        int copied4 = copyBytes(dest4, hello, 3);
        System.out.println("Test4 copied: " + copied4 + " data: " + new String(dest4, 0, copied4, StandardCharsets.UTF_8));

        // Test 5: Copy zero bytes
        byte[] dest5 = new byte[5];
        Arrays.fill(dest5, (byte) 0x41); // prefill to ensure no accidental copy
        int copied5 = copyBytes(dest5, hello, 0);
        System.out.println("Test5 copied: " + copied5 + " data: " + new String(dest5, 0, copied5, StandardCharsets.UTF_8));
    }
}