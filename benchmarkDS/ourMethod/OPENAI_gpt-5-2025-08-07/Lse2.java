import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse2 {
    // Function: copies a number of bytes from src into dest.
    // First argument: destination buffer
    // Second argument: source buffer
    // Third argument: number of bytes to copy
    // Returns the number of bytes actually copied (bounded by dest and src sizes).
    public static int safeCopyBytes(byte[] dest, byte[] src, int n) {
        if (dest == null || src == null || n <= 0) {
            return 0;
        }
        int toCopy = Math.min(n, Math.min(dest.length, src.length));
        if (toCopy <= 0) {
            return 0;
        }
        System.arraycopy(src, 0, dest, 0, toCopy);
        return toCopy;
    }

    private static void printResult(String label, byte[] buffer, int copied) {
        String content = new String(buffer, 0, copied, StandardCharsets.UTF_8);
        System.out.println(label + " bytes_copied=" + copied + ", content='" + content + "'");
    }

    public static void main(String[] args) {
        // Test 1: Copy full message to sufficiently large buffer
        byte[] message1 = "Hello, Secure World!".getBytes(StandardCharsets.UTF_8);
        byte[] buffer1 = new byte[64];
        Arrays.fill(buffer1, (byte) 0);
        int copied1 = safeCopyBytes(buffer1, message1, message1.length);
        printResult("Test 1", buffer1, copied1);

        // Test 2: Partial copy (n smaller than source and dest)
        byte[] message2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ".getBytes(StandardCharsets.UTF_8);
        byte[] buffer2 = new byte[32];
        Arrays.fill(buffer2, (byte) 0);
        int copied2 = safeCopyBytes(buffer2, message2, 5);
        printResult("Test 2", buffer2, copied2);

        // Test 3: n larger than destination capacity (should be bounded by dest)
        byte[] message3 = "1234567890abcdefghijklmnop".getBytes(StandardCharsets.UTF_8);
        byte[] buffer3 = new byte[10];
        Arrays.fill(buffer3, (byte) 0);
        int copied3 = safeCopyBytes(buffer3, message3, 50);
        printResult("Test 3", buffer3, copied3);

        // Test 4: Zero-length copy (should copy nothing)
        byte[] message4 = "NoCopy".getBytes(StandardCharsets.UTF_8);
        byte[] buffer4 = new byte[10];
        Arrays.fill(buffer4, (byte) 0);
        int copied4 = safeCopyBytes(buffer4, message4, 0);
        printResult("Test 4", buffer4, copied4);

        // Test 5: Empty source (should copy zero)
        byte[] message5 = new byte[0];
        byte[] buffer5 = new byte[10];
        Arrays.fill(buffer5, (byte) 0);
        int copied5 = safeCopyBytes(buffer5, message5, 5);
        printResult("Test 5", buffer5, copied5);
    }
}