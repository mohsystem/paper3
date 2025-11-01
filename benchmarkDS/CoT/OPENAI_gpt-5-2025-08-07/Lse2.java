import java.util.Arrays;

public class Lse2 {
    // 1) Problem understanding: copy N bytes from src to dest.
    // 2) Security requirements: null/negative checks; clamp length to buffer sizes.
    // 3) Secure coding generation: use System.arraycopy (overlap-safe).
    // 4) Code review: minimal and robust.
    // 5) Secure code output: final function below.

    // Copies up to n bytes from src to dest. Returns the number of bytes copied.
    public static int copyBytes(byte[] destination, byte[] source, int n) {
        if (destination == null || source == null) throw new IllegalArgumentException("Null buffer");
        if (n < 0) throw new IllegalArgumentException("Negative length");
        int len = Math.min(n, Math.min(destination.length, source.length));
        if (len > 0) System.arraycopy(source, 0, destination, 0, len);
        return len;
    }

    private static void printTestResult(String label, byte[] dest, int copied) {
        String shown = new String(dest, 0, Math.max(0, copied));
        System.out.println(label + " | copied=" + copied + " | result=\"" + shown + "\"");
    }

    public static void main(String[] args) {
        String msg = "Hello, World!";
        byte[] src = msg.getBytes();

        // Test 1: exact-size buffer
        byte[] dest1 = new byte[src.length];
        int c1 = copyBytes(dest1, src, src.length);
        printTestResult("Test1 (exact copy)", dest1, c1);

        // Test 2: larger buffer
        byte[] dest2 = new byte[64];
        int c2 = copyBytes(dest2, src, src.length);
        printTestResult("Test2 (into larger buffer)", Arrays.copyOf(dest2, c2), c2);

        // Test 3: partial copy (first 5 bytes)
        byte[] dest3 = new byte[5];
        int c3 = copyBytes(dest3, src, 5);
        printTestResult("Test3 (partial 5 bytes)", dest3, c3);

        // Test 4: zero-length copy
        byte[] dest4 = new byte[10];
        int c4 = copyBytes(dest4, src, 0);
        printTestResult("Test4 (zero bytes)", dest4, c4);

        // Test 5: n larger than destination (clamped)
        byte[] dest5 = new byte[4];
        int c5 = copyBytes(dest5, src, src.length);
        printTestResult("Test5 (clamped to dest size)", dest5, c5);
    }
}