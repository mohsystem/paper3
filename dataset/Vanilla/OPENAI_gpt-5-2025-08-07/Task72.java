import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Base64;

public class Task72 {
    // WARNING: Using a static IV is insecure and only for demonstration/testing.
    private static final byte[] STATIC_IV = "0123456789ABCDEF".getBytes(StandardCharsets.UTF_8);

    public static byte[] encrypt(byte[] key, byte[] data) throws Exception {
        int blockSize = 16;
        byte[] padded = padPKCS7(data, blockSize);

        SecretKeySpec keySpec = new SecretKeySpec(key, "AES");
        IvParameterSpec ivSpec = new IvParameterSpec(STATIC_IV);

        Cipher cipher = Cipher.getInstance("AES/CBC/NoPadding");
        cipher.init(Cipher.ENCRYPT_MODE, keySpec, ivSpec);

        return cipher.doFinal(padded);
    }

    private static byte[] padPKCS7(byte[] data, int blockSize) {
        int padding = blockSize - (data.length % blockSize);
        if (padding == 0) padding = blockSize;
        byte[] out = Arrays.copyOf(data, data.length + padding);
        Arrays.fill(out, data.length, out.length, (byte) padding);
        return out;
    }

    private static String toHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (byte b : bytes) sb.append(String.format("%02x", b));
        return sb.toString();
    }

    public static void main(String[] args) throws Exception {
        byte[] key = "ThisIsA16ByteKey".getBytes(StandardCharsets.UTF_8); // 16-byte key (AES-128)

        String[] tests = new String[] {
            "",
            "Hello, AES-CBC!",
            "The quick brown fox jumps over the lazy dog",
            "1234567890ABCDEF1234",
            "Data with \u0000 nulls \u0000 inside"
        };

        for (int i = 0; i < tests.length; i++) {
            byte[] plaintext = tests[i].getBytes(StandardCharsets.UTF_8);
            byte[] ciphertext = encrypt(key, plaintext);
            String b64 = Base64.getEncoder().encodeToString(ciphertext);
            System.out.println("Test " + (i + 1) + " Base64: " + b64);
            System.out.println("Test " + (i + 1) + " Hex:    " + toHex(ciphertext));
        }
    }
}