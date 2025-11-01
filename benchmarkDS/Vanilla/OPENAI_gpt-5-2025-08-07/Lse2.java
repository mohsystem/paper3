import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse2 {
    // Copies n bytes from src to dest (clamped to available lengths) and returns dest
    public static byte[] copyBytes(byte[] dest, byte[] src, int n) {
        int len = Math.min(n, Math.min(dest.length, src.length));
        for (int i = 0; i < len; i++) {
            dest[i] = src[i];
        }
        return dest;
    }

    private static void runTest(int testNum, String srcStr, int n, int destSize) {
        byte[] src = srcStr.getBytes(StandardCharsets.UTF_8);
        byte[] dest = new byte[destSize];
        Arrays.fill(dest, (byte)0);
        copyBytes(dest, src, n);
        int displayLen = Math.min(n, dest.length);
        String out = new String(dest, 0, displayLen, StandardCharsets.UTF_8);
        System.out.println("Test " + testNum + ": \"" + out + "\"");
    }

    public static void main(String[] args) {
        runTest(1, "Hello, World!", 13, 20); // full copy
        runTest(2, "Hello, World!", 5, 5);   // partial copy
        runTest(3, "Data", 4, 10);           // exact length
        runTest(4, "ByteCopy", 0, 10);       // zero bytes
        runTest(5, "Sample", 6, 6);          // exact fit
    }
}