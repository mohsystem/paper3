import java.nio.charset.StandardCharsets;

public class Lse2 {
    // Copies a number of bytes from src to dest. Returns number of bytes actually copied.
    public static int copyBytes(byte[] dest, byte[] src, int n) {
        if (dest == null || src == null || n <= 0) {
            return 0;
        }
        int count = Math.min(n, Math.min(dest.length, src.length));
        if (count > 0) {
            System.arraycopy(src, 0, dest, 0, count);
        }
        return count;
    }

    public static void main(String[] args) {
        // Test 1: Normal copy
        String msg1 = "Secure copy test";
        byte[] src1 = msg1.getBytes(StandardCharsets.UTF_8);
        byte[] dest1 = new byte[64];
        int c1 = copyBytes(dest1, src1, src1.length);
        System.out.println(new String(dest1, 0, c1, StandardCharsets.UTF_8));

        // Test 2: n larger than destination length
        String msg2 = "ShortMessage";
        byte[] src2 = msg2.getBytes(StandardCharsets.UTF_8);
        byte[] dest2 = new byte[5];
        int c2 = copyBytes(dest2, src2, 100);
        System.out.println(new String(dest2, 0, c2, StandardCharsets.UTF_8));

        // Test 3: n is zero
        String msg3 = "ZeroCopy";
        byte[] src3 = msg3.getBytes(StandardCharsets.UTF_8);
        byte[] dest3 = new byte[16];
        int c3 = copyBytes(dest3, src3, 0);
        System.out.println("Bytes copied: " + c3);

        // Test 4: n larger than source length
        String msg4 = "SrcSmall";
        byte[] src4 = msg4.getBytes(StandardCharsets.UTF_8);
        byte[] dest4 = new byte[100];
        int c4 = copyBytes(dest4, src4, 1000);
        System.out.println(new String(dest4, 0, c4, StandardCharsets.UTF_8));

        // Test 5: Exact fit
        String msg5 = "ExactFit";
        byte[] src5 = msg5.getBytes(StandardCharsets.UTF_8);
        byte[] dest5 = new byte[src5.length];
        int c5 = copyBytes(dest5, src5, src5.length);
        System.out.println(new String(dest5, 0, c5, StandardCharsets.UTF_8));
    }
}