import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.util.Arrays;

public class Task83 {

    public static byte[] encryptKeyAES_CBC(byte[] keyToEncrypt, byte[] cipherKey, byte[] iv) throws Exception {
        if (cipherKey == null || !(cipherKey.length == 16 || cipherKey.length == 24 || cipherKey.length == 32)) {
            throw new IllegalArgumentException("cipherKey must be 16, 24, or 32 bytes");
        }
        if (iv == null || iv.length != 16) {
            throw new IllegalArgumentException("iv must be 16 bytes");
        }
        Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        SecretKeySpec sk = new SecretKeySpec(cipherKey, "AES");
        IvParameterSpec ivSpec = new IvParameterSpec(iv);
        cipher.init(Cipher.ENCRYPT_MODE, sk, ivSpec);
        return cipher.doFinal(keyToEncrypt);
    }

    private static byte[] hexToBytes(String s) {
        if (s.length() % 2 != 0) throw new IllegalArgumentException("Invalid hex");
        byte[] out = new byte[s.length() / 2];
        for (int i = 0; i < out.length; i++) {
            int hi = Character.digit(s.charAt(2 * i), 16);
            int lo = Character.digit(s.charAt(2 * i + 1), 16);
            if (hi < 0 || lo < 0) throw new IllegalArgumentException("Invalid hex");
            out[i] = (byte) ((hi << 4) + lo);
        }
        return out;
    }

    private static String bytesToHex(byte[] b) {
        char[] hexArray = "0123456789abcdef".toCharArray();
        char[] hexChars = new char[b.length * 2];
        for (int j = 0; j < b.length; j++) {
            int v = b[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }

    public static void main(String[] args) throws Exception {
        // Test case 1
        byte[] key1 = hexToBytes("00112233445566778899aabbccddeeff");
        byte[] ck1 = hexToBytes("000102030405060708090a0b0c0d0e0f");
        byte[] iv1 = hexToBytes("101112131415161718191a1b1c1d1e1f");
        byte[] ct1 = encryptKeyAES_CBC(key1, ck1, iv1);
        System.out.println(bytesToHex(ct1));

        // Test case 2
        byte[] key2 = hexToBytes("000102030405060708090a0b0c0d0e0f1011121314151617");
        byte[] ck2 = hexToBytes("2b7e151628aed2a6abf7158809cf4f3c");
        byte[] iv2 = hexToBytes("000102030405060708090a0b0c0d0e0f");
        byte[] ct2 = encryptKeyAES_CBC(key2, ck2, iv2);
        System.out.println(bytesToHex(ct2));

        // Test case 3
        byte[] key3 = hexToBytes("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");
        byte[] ck3 = hexToBytes("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4");
        byte[] iv3 = hexToBytes("0f0e0d0c0b0a09080706050403020100");
        byte[] ct3 = encryptKeyAES_CBC(key3, ck3, iv3);
        System.out.println(bytesToHex(ct3));

        // Test case 4
        byte[] key4 = hexToBytes("00112233445566778899aabbccddeeff00112233");
        byte[] ck4 = hexToBytes("8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b");
        byte[] iv4 = hexToBytes("1f1e1d1c1b1a19181716151413121110");
        byte[] ct4 = encryptKeyAES_CBC(key4, ck4, iv4);
        System.out.println(bytesToHex(ct4));

        // Test case 5
        byte[] key5 = hexToBytes("000102030405060708090a0b0c0d0e");
        byte[] ck5 = hexToBytes("000102030405060708090a0b0c0d0e0f");
        byte[] iv5 = hexToBytes("aabbccddeeff00112233445566778899");
        byte[] ct5 = encryptKeyAES_CBC(key5, ck5, iv5);
        System.out.println(bytesToHex(ct5));
    }
}