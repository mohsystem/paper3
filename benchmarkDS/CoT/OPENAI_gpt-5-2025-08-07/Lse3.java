import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse3 {

    // Step 1-5: Secure byte copy with validation and overlap-safety using a temporary buffer.
    public static int copyBytes(byte[] dest, byte[] src, int n) {
        if (dest == null || src == null || n <= 0) {
            return 0;
        }
        int bytesToCopy = Math.min(n, Math.min(dest.length, src.length));
        if (bytesToCopy <= 0) {
            return 0;
        }
        byte[] temp = Arrays.copyOf(src, bytesToCopy);
        System.arraycopy(temp, 0, dest, 0, bytesToCopy);
        return bytesToCopy;
        // End of secure copy
    }

    private static String toStringBytes(byte[] arr, int count) {
        if (arr == null || count <= 0) return "";
        int c = Math.min(count, arr.length);
        return new String(arr, 0, c, StandardCharsets.UTF_8);
    }

    public static void main(String[] args) {
        // Test 1: Copy "Hello" (5 bytes)
        byte[] dest1 = new byte[16];
        byte[] src1 = "Hello".getBytes(StandardCharsets.UTF_8);
        int c1 = copyBytes(dest1, src1, 5);
        System.out.println("Test1 bytesCopied=" + c1 + " content=\"" + toStringBytes(dest1, c1) + "\"");

        // Test 2: Request more than source length (10) - should clamp to 5
        byte[] dest2 = new byte[16];
        int c2 = copyBytes(dest2, src1, 10);
        System.out.println("Test2 bytesCopied=" + c2 + " content=\"" + toStringBytes(dest2, c2) + "\"");

        // Test 3: Destination smaller than requested - should clamp to dest length
        byte[] dest3 = new byte[3];
        int c3 = copyBytes(dest3, src1, 5);
        System.out.println("Test3 bytesCopied=" + c3 + " content=\"" + toStringBytes(dest3, c3) + "\"");

        // Test 4: Zero length copy
        byte[] dest4 = new byte[10];
        int c4 = copyBytes(dest4, src1, 0);
        System.out.println("Test4 bytesCopied=" + c4 + " content=\"" + toStringBytes(dest4, c4) + "\"");

        // Test 5: Null source
        byte[] dest5 = new byte[10];
        int c5 = copyBytes(dest5, null, 5);
        System.out.println("Test5 bytesCopied=" + c5 + " content=\"" + toStringBytes(dest5, c5) + "\"");
    }
}