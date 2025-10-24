import javax.crypto.Cipher;
import javax.crypto.Mac;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.security.SecureRandom;
import java.util.Arrays;

public class Task83 {

    public static byte[] encryptKey(byte[] encKey, byte[] macKey, byte[] keyToEncrypt) throws Exception {
        if (!isValidAesKey(encKey)) {
            throw new IllegalArgumentException("Encryption key must be 16, 24, or 32 bytes.");
        }
        if (macKey == null || macKey.length == 0) {
            throw new IllegalArgumentException("MAC key must not be empty.");
        }
        if (keyToEncrypt == null) {
            throw new IllegalArgumentException("Key to encrypt must not be null.");
        }

        byte[] iv = new byte[16];
        SecureRandom sr = SecureRandom.getInstanceStrong();
        sr.nextBytes(iv);

        SecretKeySpec aesKey = new SecretKeySpec(encKey, "AES");
        IvParameterSpec ivSpec = new IvParameterSpec(iv);

        Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        cipher.init(Cipher.ENCRYPT_MODE, aesKey, ivSpec);
        byte[] ciphertext = cipher.doFinal(keyToEncrypt);

        Mac mac = Mac.getInstance("HmacSHA256");
        mac.init(new SecretKeySpec(macKey, "HmacSHA256"));
        mac.update(iv);
        mac.update(ciphertext);
        byte[] tag = mac.doFinal();

        byte[] out = new byte[iv.length + ciphertext.length + tag.length];
        System.arraycopy(iv, 0, out, 0, iv.length);
        System.arraycopy(ciphertext, 0, out, iv.length, ciphertext.length);
        System.arraycopy(tag, 0, out, iv.length + ciphertext.length, tag.length);
        return out;
    }

    private static boolean isValidAesKey(byte[] k) {
        if (k == null) return false;
        int n = k.length;
        return n == 16 || n == 24 || n == 32;
    }

    private static byte[] hexToBytes(String s) {
        if (s.length() % 2 != 0) throw new IllegalArgumentException("Hex string must have even length");
        byte[] out = new byte[s.length() / 2];
        for (int i = 0; i < out.length; i++) {
            int hi = Character.digit(s.charAt(2 * i), 16);
            int lo = Character.digit(s.charAt(2 * i + 1), 16);
            if (hi < 0 || lo < 0) throw new IllegalArgumentException("Invalid hex");
            out[i] = (byte) ((hi << 4) | lo);
        }
        return out;
    }

    private static String toHex(byte[] data) {
        char[] hexArray = "0123456789abcdef".toCharArray();
        char[] hexChars = new char[data.length * 2];
        for (int j = 0; j < data.length; j++) {
            int v = data[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }

    public static void main(String[] args) throws Exception {
        String encKeyHex = "603deb1015ca71be2b73aef0857d7781f352c073b6108d72d9810a30914dff4";
        String macKeyHex = "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f";
        byte[] encKey = hexToBytes(encKeyHex);
        byte[] macKey = hexToBytes(macKeyHex);

        String[] testKeysHex = new String[] {
                "00112233445566778899aabbccddeeff",
                "000102030405060708090a0b0c0d0e0f1011121314151617",
                "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
                "deadbeef",
                "aabbccddeeff"
        };

        for (int i = 0; i < testKeysHex.length; i++) {
            byte[] keyToEncrypt = hexToBytes(testKeysHex[i]);
            byte[] result = encryptKey(encKey, macKey, keyToEncrypt);
            System.out.println("Test " + (i + 1) + " (iv|ciphertext|tag hex): " + toHex(result));
        }
    }
}