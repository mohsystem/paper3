// Chain-of-Through secure implementation for AES-CBC encryption.
// 1) Problem understanding: Implement AES-CBC to encrypt a provided key using a provided AES key.
// 2) Security requirements: Use strong randomness for IV, validate key sizes, use standard padding, avoid static IVs.
// 3) Secure coding generation: Implement with robust checks and safe APIs.
// 4) Code review: Keep error handling and validation; avoid weak randomness; include per-operation IV.
// 5) Secure code output: Return Base64(IV || ciphertext).

import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.util.Base64;

public class Task83 {

    public static String encryptKey(byte[] aesKey, byte[] keyToEncrypt) throws GeneralSecurityException {
        if (aesKey == null || keyToEncrypt == null) {
            throw new IllegalArgumentException("Inputs must not be null");
        }
        if (!(aesKey.length == 16 || aesKey.length == 24 || aesKey.length == 32)) {
            throw new IllegalArgumentException("AES key must be 16, 24, or 32 bytes");
        }

        byte[] iv = new byte[16];
        SecureRandom sr;
        try {
            sr = SecureRandom.getInstanceStrong();
        } catch (Exception e) {
            sr = new SecureRandom();
        }
        sr.nextBytes(iv);

        SecretKeySpec keySpec = new SecretKeySpec(aesKey, "AES");
        IvParameterSpec ivSpec = new IvParameterSpec(iv);

        Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        cipher.init(Cipher.ENCRYPT_MODE, keySpec, ivSpec);
        byte[] ciphertext = cipher.doFinal(keyToEncrypt);

        byte[] out = new byte[iv.length + ciphertext.length];
        System.arraycopy(iv, 0, out, 0, iv.length);
        System.arraycopy(ciphertext, 0, out, iv.length, ciphertext.length);

        return Base64.getEncoder().encodeToString(out);
    }

    // Helper for tests: hex decode
    private static byte[] hexToBytes(String hex) {
        if (hex == null || (hex.length() % 2) != 0) throw new IllegalArgumentException("Invalid hex");
        int len = hex.length();
        byte[] out = new byte[len / 2];
        for (int i = 0; i < out.length; i++) {
            int hi = Character.digit(hex.charAt(2 * i), 16);
            int lo = Character.digit(hex.charAt(2 * i + 1), 16);
            if (hi < 0 || lo < 0) throw new IllegalArgumentException("Invalid hex char");
            out[i] = (byte) ((hi << 4) + lo);
        }
        return out;
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases demonstrating AES-128/192/256 with different data
        byte[] k128 = hexToBytes("00112233445566778899aabbccddeeff");
        byte[] k192 = hexToBytes("00112233445566778899aabbccddeeff0011223344556677");
        byte[] k256 = hexToBytes("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");

        String c1 = encryptKey(k128, "secret-key-1".getBytes(StandardCharsets.UTF_8));
        String c2 = encryptKey(k192, "another-secret-key-2".getBytes(StandardCharsets.UTF_8));
        String c3 = encryptKey(k256, "K3".getBytes(StandardCharsets.UTF_8));
        String c4 = encryptKey(k128, new byte[]{0x00, 0x01, 0x02, 0x03, (byte) 0xFF});
        String c5 = encryptKey(k256, "A much longer piece of data to encrypt as key material.".getBytes(StandardCharsets.UTF_8));

        System.out.println("Test1: " + c1);
        System.out.println("Test2: " + c2);
        System.out.println("Test3: " + c3);
        System.out.println("Test4: " + c4);
        System.out.println("Test5: " + c5);
    }
}