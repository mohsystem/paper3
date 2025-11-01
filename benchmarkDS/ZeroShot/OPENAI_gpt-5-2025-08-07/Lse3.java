import java.nio.charset.StandardCharsets;

public class Lse3 {
    // Copy n bytes from src to dest. Returns number of bytes copied, or -1 on error.
    public static int copyBytes(byte[] dest, byte[] src, int n) {
        if (n < 0) return -1;
        if (n == 0) return 0;
        if (dest == null || src == null) return -1;
        if (n > dest.length || n > src.length) return -1;

        // Manual copy to avoid surprises; forward copy is fine as these are separate arrays.
        for (int i = 0; i < n; i++) {
            dest[i] = src[i];
        }
        return n;
    }

    public static void main(String[] args) {
        // Test 1: Normal copy of "Hello"
        byte[] src1 = "Hello".getBytes(StandardCharsets.UTF_8);
        byte[] dest1 = new byte[10];
        int r1 = copyBytes(dest1, src1, 5);
        System.out.println("Test1 result=" + r1 + " dest='" + new String(dest1, 0, 5, StandardCharsets.UTF_8) + "'");

        // Test 2: Dest too small (should fail)
        byte[] dest2 = new byte[3];
        int r2 = copyBytes(dest2, src1, 5);
        System.out.println("Test2 result=" + r2);

        // Test 3: Exact size buffer for "Hello"
        byte[] dest3 = new byte[5];
        int r3 = copyBytes(dest3, src1, 5);
        System.out.println("Test3 result=" + r3 + " dest='" + new String(dest3, 0, r3, StandardCharsets.UTF_8) + "'");

        // Test 4: Copy 0 bytes (no-op)
        byte[] dest4 = new byte[2];
        int r4 = copyBytes(dest4, src1, 0);
        System.out.println("Test4 result=" + r4);

        // Test 5: Null source (should fail)
        int r5 = copyBytes(new byte[10], null, 5);
        System.out.println("Test5 result=" + r5);
    }
}