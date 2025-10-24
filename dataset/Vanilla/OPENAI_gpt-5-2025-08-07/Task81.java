import java.security.MessageDigest;
import java.util.Arrays;
import java.nio.charset.StandardCharsets;

public class Task81 {

    public static boolean matchesKnownHash(byte[] certBytes, String knownHashHex, String algorithm) {
        try {
            MessageDigest md = MessageDigest.getInstance(algorithm);
            byte[] digest = md.digest(certBytes);
            String hex = bytesToHex(digest);
            return hex.equalsIgnoreCase(knownHashHex);
        } catch (Exception e) {
            return false;
        }
    }

    private static String bytesToHex(byte[] bytes) {
        char[] HEX = "0123456789abcdef".toCharArray();
        char[] out = new char[bytes.length * 2];
        int i = 0;
        for (byte b : bytes) {
            out[i++] = HEX[(b >>> 4) & 0x0F];
            out[i++] = HEX[b & 0x0F];
        }
        return new String(out);
    }

    public static void main(String[] args) {
        // 5 test cases using SHA-256
        String algo = "SHA-256";

        byte[] cert1 = "abc".getBytes(StandardCharsets.UTF_8);
        String hash1 = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
        System.out.println(matchesKnownHash(cert1, hash1, algo)); // true

        byte[] cert2 = "".getBytes(StandardCharsets.UTF_8);
        String hash2 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
        System.out.println(matchesKnownHash(cert2, hash2, algo)); // true

        byte[] cert3 = "hello".getBytes(StandardCharsets.UTF_8);
        String hash3 = "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824";
        System.out.println(matchesKnownHash(cert3, hash3, algo)); // true

        byte[] cert4 = "test".getBytes(StandardCharsets.UTF_8);
        String hash4Wrong = "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";
        System.out.println(matchesKnownHash(cert4, hash4Wrong, algo)); // false

        byte[] cert5 = new byte[] {0x00, 0x01, 0x02, 0x03};
        System.out.println(matchesKnownHash(cert5, hash1, algo)); // false
    }
}