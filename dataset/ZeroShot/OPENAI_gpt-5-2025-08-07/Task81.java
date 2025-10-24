import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Base64;

public class Task81 {

    public static boolean certificateMatchesHash(byte[] certData, String expectedSha256Hex) {
        if (certData == null || expectedSha256Hex == null) return false;
        byte[] expected = decodeHex(sanitizeHex(expectedSha256Hex));
        if (expected == null || expected.length != 32) return false;
        byte[] actual = sha256(certData);
        if (actual == null) return false;
        return constantTimeEquals(actual, expected);
    }

    public static boolean certificateMatchesHashFromPem(String pem, String expectedSha256Hex) {
        if (pem == null || expectedSha256Hex == null) return false;
        byte[] der = pemToDer(pem);
        if (der == null) return false;
        return certificateMatchesHash(der, expectedSha256Hex);
    }

    private static byte[] sha256(byte[] data) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            return md.digest(data);
        } catch (NoSuchAlgorithmException e) {
            return null;
        }
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null || a.length != b.length) return false;
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= (a[i] ^ b[i]);
        }
        return result == 0;
    }

    private static String sanitizeHex(String s) {
        return s.replaceAll("[^0-9A-Fa-f]", "");
    }

    private static byte[] decodeHex(String hex) {
        if (hex == null || (hex.length() & 1) != 0) return null;
        int len = hex.length();
        byte[] out = new byte[len / 2];
        for (int i = 0, j = 0; i < len; i += 2, j++) {
            int hi = Character.digit(hex.charAt(i), 16);
            int lo = Character.digit(hex.charAt(i + 1), 16);
            if (hi == -1 || lo == -1) return null;
            out[j] = (byte) ((hi << 4) | lo);
        }
        return out;
    }

    private static byte[] pemToDer(String pem) {
        String header = "-----BEGIN CERTIFICATE-----";
        String footer = "-----END CERTIFICATE-----";
        int h = pem.indexOf(header);
        int f = pem.indexOf(footer);
        if (h < 0 || f < 0 || f <= h) return null;
        String base64Body = pem.substring(h + header.length(), f);
        // Base64 MIME decoder tolerates whitespace/newlines
        try {
            return Base64.getMimeDecoder().decode(base64Body);
        } catch (IllegalArgumentException e) {
            return null;
        }
    }

    public static void main(String[] args) {
        String hello = "hello";
        byte[] helloBytes = hello.getBytes(StandardCharsets.UTF_8);
        String shaHello = "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824";
        String wrong64zeros = "0000000000000000000000000000000000000000000000000000000000000000";

        String pemHello = "-----BEGIN CERTIFICATE-----\n" +
                "aGVsbG8=\n" +
                "-----END CERTIFICATE-----\n";

        String pemHelloMessy = "-----BEGIN CERTIFICATE-----\n" +
                "  aG Vs\n" +
                "  bG8=\n" +
                "-----END CERTIFICATE-----";

        String invalidPem = "-----BEGIN CERTIFICATE-----\n" +
                "???not_base64@@@\n" +
                "-----END CERTIFICATE-----";

        boolean t1 = certificateMatchesHash(helloBytes, shaHello);
        boolean t2 = certificateMatchesHash(helloBytes, wrong64zeros);
        boolean t3 = certificateMatchesHashFromPem(pemHello, shaHello);
        boolean t4 = certificateMatchesHashFromPem(pemHelloMessy, shaHello);
        boolean t5 = certificateMatchesHashFromPem(invalidPem, shaHello);

        System.out.println("Test1 (DER correct): " + t1);
        System.out.println("Test2 (DER wrong):   " + t2);
        System.out.println("Test3 (PEM correct): " + t3);
        System.out.println("Test4 (PEM messy):   " + t4);
        System.out.println("Test5 (PEM invalid): " + t5);
    }
}