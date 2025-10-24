import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.util.Arrays;
import java.util.Base64;

public class Task72 {
    private static final byte[] STATIC_IV = new byte[] {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F
    };
    private static final int BLOCK_SIZE = 16;

    public static byte[] pkcs7Pad(byte[] data, int blockSize) {
        if (blockSize <= 0) throw new IllegalArgumentException("Invalid block size");
        int padLen = blockSize - (data.length % blockSize);
        if (padLen == 0) padLen = blockSize;
        byte[] out = Arrays.copyOf(data, data.length + padLen);
        Arrays.fill(out, data.length, out.length, (byte) padLen);
        return out;
    }

    public static byte[] encrypt(byte[] key, byte[] data) throws GeneralSecurityException {
        if (key == null || data == null) throw new IllegalArgumentException("Key/data is null");
        if (!(key.length == 16 || key.length == 24 || key.length == 32)) {
            throw new IllegalArgumentException("AES key length must be 16, 24, or 32 bytes");
        }
        byte[] padded = pkcs7Pad(data, BLOCK_SIZE);
        SecretKeySpec keySpec = new SecretKeySpec(key, "AES");
        IvParameterSpec ivSpec = new IvParameterSpec(STATIC_IV);
        Cipher cipher = Cipher.getInstance("AES/CBC/NoPadding");
        cipher.init(Cipher.ENCRYPT_MODE, keySpec, ivSpec);
        return cipher.doFinal(padded);
    }

    private static String b64(byte[] bytes) {
        return Base64.getEncoder().encodeToString(bytes);
    }

    public static void main(String[] args) {
        try {
            byte[] key = "0123456789ABCDEF".getBytes(StandardCharsets.UTF_8); // 16-byte key
            String[] tests = new String[] {
                "",
                "Hello, World!",
                "The quick brown fox jumps over the lazy dog",
                "1234567890abcdef",
                "A longer message that will span multiple blocks to test padding and encryption correctness."
            };
            for (int i = 0; i < tests.length; i++) {
                byte[] ct = encrypt(key, tests[i].getBytes(StandardCharsets.UTF_8));
                System.out.println("Test " + (i+1) + ": " + b64(ct));
            }
        } catch (Exception e) {
            System.err.println("Encryption error: " + e.getMessage());
        }
    }
}