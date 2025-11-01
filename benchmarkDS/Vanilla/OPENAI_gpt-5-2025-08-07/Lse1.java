import java.nio.charset.StandardCharsets;

public class Lse1 {
    // Copies n bytes from src to dest; first arg dest, second src, third n
    public static void memCopy(byte[] dest, byte[] src, int n) {
        if (dest == null || src == null) throw new IllegalArgumentException("Null array");
        if (n < 0) throw new IllegalArgumentException("Negative length");
        if (n > dest.length || n > src.length) throw new IllegalArgumentException("Length exceeds array bounds");
        for (int i = 0; i < n; i++) {
            dest[i] = src[i];
        }
    }

    private static String hex(byte[] a, int n) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < n; i++) {
            sb.append(String.format("%02X", a[i] & 0xFF));
            if (i + 1 < n) sb.append(" ");
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        // Test 1: Copy full ASCII buffer
        byte[] src1 = "HelloWorld".getBytes(StandardCharsets.UTF_8);
        byte[] dest1 = new byte[10];
        memCopy(dest1, src1, 10);
        System.out.println("T1 HEX: " + hex(dest1, 10));
        System.out.println("T1 STR: " + new String(dest1, StandardCharsets.UTF_8));

        // Test 2: Partial copy
        byte[] src2 = "OpenAI".getBytes(StandardCharsets.UTF_8);
        byte[] dest2 = new byte[4];
        memCopy(dest2, src2, 4); // "Open"
        System.out.println("T2 HEX: " + hex(dest2, 4));
        System.out.println("T2 STR: " + new String(dest2, StandardCharsets.UTF_8));

        // Test 3: Zero-length copy (dest remains unchanged)
        byte[] src3 = "XYZ".getBytes(StandardCharsets.UTF_8);
        byte[] dest3 = new byte[] {(byte)0xEE, (byte)0xEE, (byte)0xEE};
        memCopy(dest3, src3, 0);
        System.out.println("T3 HEX: " + hex(dest3, dest3.length));

        // Test 4: Binary data copy
        byte[] src4 = new byte[] {0x00, 0x01, (byte)0xAB, (byte)0xFF, 0x10};
        byte[] dest4 = new byte[5];
        memCopy(dest4, src4, 5);
        System.out.println("T4 HEX: " + hex(dest4, 5));

        // Test 5: Copy first 7 bytes
        byte[] src5 = "1234567890".getBytes(StandardCharsets.UTF_8);
        byte[] dest5 = new byte[7];
        memCopy(dest5, src5, 7); // "1234567"
        System.out.println("T5 HEX: " + hex(dest5, 7));
        System.out.println("T5 STR: " + new String(dest5, StandardCharsets.UTF_8));
    }
}