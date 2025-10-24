import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Task51 {

    // Encrypts plaintext using XXTEA with a 16-byte hex key (32 hex chars). Returns ciphertext hex string.
    public static String encrypt(String plaintext, String keyHex) {
        if (plaintext == null || keyHex == null) return "";
        int[] key = parseHexKey(keyHex);
        if (key == null) return "";
        byte[] data = plaintext.getBytes(StandardCharsets.UTF_8);
        if (data.length == 0) return "";
        int[] v = toUint32Array(data, true);
        xxteaEncrypt(v, key);
        byte[] out = toByteArray(v, false); // full block bytes
        return bytesToHex(out);
    }

    // Decrypts ciphertext hex string using XXTEA with a 16-byte hex key (32 hex chars). Returns plaintext.
    public static String decrypt(String cipherHex, String keyHex) {
        if (cipherHex == null || keyHex == null) return "";
        int[] key = parseHexKey(keyHex);
        if (key == null) return "";
        byte[] enc = hexToBytes(cipherHex);
        if (enc == null || enc.length == 0) return "";
        int[] v = toUint32Array(enc, false);
        if (v.length == 0) return "";
        xxteaDecrypt(v, key);
        byte[] out = toByteArray(v, true);
        if (out == null) return "";
        return new String(out, StandardCharsets.UTF_8);
    }

    private static int[] parseHexKey(String keyHex) {
        if (keyHex.length() != 32) return null;
        byte[] kb = hexToBytes(keyHex);
        if (kb == null || kb.length != 16) return null;
        int[] k = new int[4];
        for (int i = 0; i < 4; i++) {
            int j = i * 4;
            k[i] = ((kb[j] & 0xFF)) |
                   ((kb[j + 1] & 0xFF) << 8) |
                   ((kb[j + 2] & 0xFF) << 16) |
                   ((kb[j + 3] & 0xFF) << 24);
        }
        return k;
    }

    private static int[] toUint32Array(byte[] data, boolean includeLength) {
        int n = (data.length + 3) >>> 2;
        int[] result = includeLength ? new int[n + 1] : new int[n == 0 ? 1 : n];
        Arrays.fill(result, 0);
        int i = 0;
        for (; i < data.length; i++) {
            result[i >>> 2] |= (data[i] & 0xFF) << ((i & 3) << 3);
        }
        if (includeLength) {
            result[result.length - 1] = data.length;
        }
        return result;
    }

    private static byte[] toByteArray(int[] data, boolean includeLength) {
        int n = data.length << 2;
        int m = n;
        if (includeLength) {
            if (data.length == 0) return new byte[0];
            int len = data[data.length - 1];
            if (len < 0 || len > n) return null;
            m = len;
        }
        byte[] result = new byte[m];
        for (int i = 0; i < m; i++) {
            result[i] = (byte) ((data[i >>> 2] >>> ((i & 3) << 3)) & 0xFF);
        }
        return result;
    }

    private static void xxteaEncrypt(int[] v, int[] k) {
        int n = v.length;
        if (n < 2) return;
        int rounds = 6 + 52 / n;
        int sum = 0;
        int z = v[n - 1], y;
        final int DELTA = 0x9E3779B9;
        while (rounds-- > 0) {
            sum += DELTA;
            int e = (sum >>> 2) & 3;
            for (int p = 0; p < n - 1; p++) {
                y = v[p + 1];
                int mx = (((z >>> 5) ^ (y << 2)) + ((y >>> 3) ^ (z << 4))) ^ ((sum ^ y) + (k[(p & 3) ^ e] ^ z));
                z = v[p] += mx;
            }
            y = v[0];
            int mx = (((z >>> 5) ^ (y << 2)) + ((y >>> 3) ^ (z << 4))) ^ ((sum ^ y) + (k[((n - 1) & 3) ^ e] ^ z));
            z = v[n - 1] += mx;
        }
    }

    private static void xxteaDecrypt(int[] v, int[] k) {
        int n = v.length;
        if (n < 2) return;
        final int DELTA = 0x9E3779B9;
        int rounds = 6 + 52 / n;
        int sum = rounds * DELTA;
        int y, z = v[n - 1];
        while (sum != 0) {
            int e = (sum >>> 2) & 3;
            for (int p = n - 1; p > 0; p--) {
                z = v[p - 1];
                int mx = (((z >>> 5) ^ (v[p] << 2)) + ((v[p] >>> 3) ^ (z << 4))) ^ ((sum ^ v[p]) + (k[(p & 3) ^ e] ^ z));
                v[p] -= mx;
            }
            y = v[0];
            int mx = (((z >>> 5) ^ (y << 2)) + ((y >>> 3) ^ (z << 4))) ^ ((sum ^ y) + (k[(0 & 3) ^ e] ^ z));
            v[0] -= mx;
            z = v[n - 1];
            sum -= DELTA;
        }
    }

    private static String bytesToHex(byte[] data) {
        char[] hexArray = "0123456789abcdef".toCharArray();
        char[] out = new char[data.length * 2];
        for (int i = 0; i < data.length; i++) {
            int v = data[i] & 0xFF;
            out[i * 2] = hexArray[v >>> 4];
            out[i * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(out);
    }

    private static byte[] hexToBytes(String hex) {
        String s = hex.trim();
        if ((s.length() & 1) != 0) return null;
        int len = s.length() / 2;
        byte[] out = new byte[len];
        for (int i = 0; i < len; i++) {
            int hi = Character.digit(s.charAt(2 * i), 16);
            int lo = Character.digit(s.charAt(2 * i + 1), 16);
            if (hi < 0 || lo < 0) return null;
            out[i] = (byte) ((hi << 4) | lo);
        }
        return out;
    }

    public static void main(String[] args) {
        String key = "00112233445566778899aabbccddeeff";
        String[] tests = new String[] {
            "",
            "hello",
            "The quick brown fox jumps over the lazy dog.",
            "1234567890",
            "Sensitive data!"
        };
        for (String t : tests) {
            String enc = encrypt(t, key);
            String dec = decrypt(enc, key);
            System.out.println("PT: " + t);
            System.out.println("CT: " + enc);
            System.out.println("DC: " + dec);
            System.out.println("----");
        }
    }
}