import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Task72 {
    // Static IV - for demonstration only; avoid static IVs in real systems.
    private static final byte[] STATIC_IV = new byte[] {
            0x00, 0x11, 0x22, 0x33,
            0x44, 0x55, 0x66, 0x77,
            (byte) 0x88, (byte) 0x99, (byte) 0xAA, (byte) 0xBB,
            (byte) 0xCC, (byte) 0xDD, (byte) 0xEE, (byte) 0xFF
    };

    public static byte[] encryptAesCbc(byte[] key, byte[] plaintext) throws Exception {
        if (key == null || (! (key.length == 16 || key.length == 24 || key.length == 32))) {
            throw new IllegalArgumentException("Key length must be 16, 24, or 32 bytes");
        }
        byte[] padded = padPKCS7(plaintext, 16);
        SecretKeySpec skey = new SecretKeySpec(key, "AES");
        IvParameterSpec iv = new IvParameterSpec(STATIC_IV);
        Cipher cipher = Cipher.getInstance("AES/CBC/NoPadding");
        cipher.init(Cipher.ENCRYPT_MODE, skey, iv);
        return cipher.doFinal(padded);
    }

    private static byte[] padPKCS7(byte[] data, int blockSize) {
        if (blockSize <= 0) throw new IllegalArgumentException("Invalid block size");
        int padLen = blockSize - (data.length % blockSize);
        if (padLen == 0) padLen = blockSize;
        byte[] out = Arrays.copyOf(data, data.length + padLen);
        Arrays.fill(out, data.length, out.length, (byte) padLen);
        return out;
    }

    private static String toHex(byte[] b) {
        StringBuilder sb = new StringBuilder(b.length * 2);
        for (byte value : b) {
            sb.append(String.format("%02x", value & 0xff));
        }
        return sb.toString();
    }

    public static void main(String[] args) throws Exception {
        // Test case 1
        byte[] key1 = new byte[] {
                0x00, 0x01, 0x02, 0x03,
                0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0A, 0x0B,
                0x0C, 0x0D, 0x0E, 0x0F
        };
        byte[] pt1 = new byte[0];
        System.out.println("TC1: " + toHex(encryptAesCbc(key1, pt1)));

        // Test case 2
        byte[] key2 = new byte[16];
        Arrays.fill(key2, (byte) 0x01);
        byte[] pt2 = "A".getBytes(StandardCharsets.UTF_8);
        System.out.println("TC2: " + toHex(encryptAesCbc(key2, pt2)));

        // Test case 3
        byte[] key3 = "Sixteen byte key".getBytes(StandardCharsets.UTF_8); // 16 bytes
        byte[] pt3 = "Hello World".getBytes(StandardCharsets.UTF_8);
        System.out.println("TC3: " + toHex(encryptAesCbc(key3, pt3)));

        // Test case 4 (AES-192)
        byte[] key4 = new byte[24];
        for (int i = 0; i < key4.length; i++) key4[i] = (byte) i;
        byte[] pt4 = "The quick brown fox jumps over the lazy dog".getBytes(StandardCharsets.UTF_8);
        System.out.println("TC4: " + toHex(encryptAesCbc(key4, pt4)));

        // Test case 5 (AES-256)
        byte[] key5 = new byte[32];
        for (int i = 0; i < key5.length; i++) key5[i] = (byte) i;
        byte[] pt5 = "Data with a length not multiple of block size.".getBytes(StandardCharsets.UTF_8);
        System.out.println("TC5: " + toHex(encryptAesCbc(key5, pt5)));
    }
}