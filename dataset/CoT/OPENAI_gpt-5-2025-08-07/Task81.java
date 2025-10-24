import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Base64;

public class Task81 {

    // Normalize and validate hashing algorithm
    private static String normalizeAlg(String alg) {
        if (alg == null) throw new IllegalArgumentException("Algorithm cannot be null.");
        String up = alg.trim().toUpperCase();
        switch (up) {
            case "SHA-256":
            case "SHA256":
                return "SHA-256";
            case "SHA-384":
            case "SHA384":
                return "SHA-384";
            case "SHA-512":
            case "SHA512":
                return "SHA-512";
            default:
                throw new IllegalArgumentException("Unsupported algorithm. Allowed: SHA-256, SHA-384, SHA-512");
        }
    }

    // Convert hex string (with optional colons/spaces) to bytes
    private static byte[] hexToBytes(String hex) {
        if (hex == null) throw new IllegalArgumentException("Expected hex cannot be null.");
        StringBuilder sb = new StringBuilder();
        for (char c : hex.toCharArray()) {
            if (Character.digit(c, 16) != -1) {
                sb.append(Character.toUpperCase(c));
            }
        }
        String cleaned = sb.toString();
        if (cleaned.length() == 0 || (cleaned.length() % 2 != 0)) {
            throw new IllegalArgumentException("Invalid hex fingerprint format.");
        }
        int len = cleaned.length() / 2;
        byte[] out = new byte[len];
        for (int i = 0; i < len; i++) {
            int hi = Character.digit(cleaned.charAt(i * 2), 16);
            int lo = Character.digit(cleaned.charAt(i * 2 + 1), 16);
            if (hi == -1 || lo == -1) throw new IllegalArgumentException("Invalid hex digit in fingerprint.");
            out[i] = (byte) ((hi << 4) | lo);
        }
        return out;
    }

    // Convert bytes to hex uppercase (no delimiters)
    private static String bytesToHex(byte[] data) {
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) {
            sb.append(String.format("%02X", b));
        }
        return sb.toString();
    }

    // Extract DER bytes from PEM or return null if not PEM
    private static byte[] pemToDerIfPem(String s) {
        if (s == null) return null;
        String markerBegin = "-----BEGIN CERTIFICATE-----";
        String markerEnd = "-----END CERTIFICATE-----";
        int start = s.indexOf(markerBegin);
        int end = s.indexOf(markerEnd);
        if (start == -1 || end == -1 || end < start) {
            return null;
        }
        int base64Start = start + markerBegin.length();
        String base64Content = s.substring(base64Start, end);
        // Decode with MIME decoder to ignore whitespace/newlines
        return Base64.getMimeDecoder().decode(base64Content);
    }

    // Compute digest bytes
    private static byte[] computeDigest(byte[] der, String alg) {
        try {
            MessageDigest md = MessageDigest.getInstance(normalizeAlg(alg));
            return md.digest(der);
        } catch (NoSuchAlgorithmException e) {
            throw new IllegalArgumentException("Hash algorithm not available.", e);
        }
    }

    // Public helper: compute hex digest from DER bytes
    public static String computeCertDigestHex(byte[] der, String alg) {
        if (der == null) throw new IllegalArgumentException("Certificate data cannot be null.");
        return bytesToHex(computeDigest(der, alg));
        }

    // Constant-time comparison
    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        return MessageDigest.isEqual(a, b);
    }

    // Main API: DER or raw bytes with expected hex fingerprint and algorithm
    public static boolean certificateMatchesHash(byte[] certBytes, String expectedHex, String alg) {
        if (certBytes == null) throw new IllegalArgumentException("Certificate data cannot be null.");
        byte[] expected = hexToBytes(expectedHex);
        byte[] actual = computeDigest(certBytes, alg);
        return constantTimeEquals(actual, expected);
    }

    // Main API: PEM string with expected hex fingerprint and algorithm
    public static boolean certificateMatchesHashFromPem(String pem, String expectedHex, String alg) {
        byte[] der = pemToDerIfPem(pem);
        if (der == null) throw new IllegalArgumentException("Invalid PEM format or certificate block not found.");
        return certificateMatchesHash(der, expectedHex, alg);
    }

    // Utility to insert colons to hex fingerprint (AA:BB:...)
    private static String colonizeHex(String hex) {
        StringBuilder sb = new StringBuilder();
        String cleaned = hex.replaceAll("[^0-9A-Fa-f]", "").toUpperCase();
        for (int i = 0; i < cleaned.length(); i += 2) {
            if (i > 0) sb.append(":");
            sb.append(cleaned.charAt(i));
            if (i + 1 < cleaned.length()) sb.append(cleaned.charAt(i + 1));
        }
        return sb.toString();
    }

    // Base64 encode helper for tests
    private static String base64Encode(byte[] data, int lineWidth) {
        String enc = Base64.getMimeEncoder(lineWidth, "\n".getBytes(StandardCharsets.US_ASCII)).encodeToString(data);
        return enc;
    }

    public static void main(String[] args) {
        // Test 1: DER bytes match using SHA-256
        byte[] cert1 = "DERCERT-ONE".getBytes(StandardCharsets.UTF_8);
        String expected1 = computeCertDigestHex(cert1, "SHA-256");
        System.out.println("Test1 (DER match, SHA-256): " + certificateMatchesHash(cert1, expected1, "SHA-256"));

        // Test 2: DER bytes do not match (wrong expected)
        byte[] cert2 = "ANOTHER-CERT".getBytes(StandardCharsets.UTF_8);
        String wrongExpected = "00".repeat(32); // 64 hex chars (256-bit)
        System.out.println("Test2 (DER mismatch): " + certificateMatchesHash(cert2, wrongExpected, "SHA-256"));

        // Test 3: PEM certificate match using SHA-256
        byte[] der3 = new byte[]{1, 2, 3, 4, 5};
        String pemBody = base64Encode(der3, 64);
        String pem = "-----BEGIN CERTIFICATE-----\n" + pemBody + "\n-----END CERTIFICATE-----\n";
        String expected3 = computeCertDigestHex(der3, "SHA-256");
        System.out.println("Test3 (PEM match, SHA-256): " + certificateMatchesHashFromPem(pem, expected3, "SHA-256"));

        // Test 4: Invalid algorithm handling
        try {
            certificateMatchesHash(cert1, expected1, "MD5");
            System.out.println("Test4 (Invalid alg): false"); // shouldn't reach here
        } catch (IllegalArgumentException e) {
            System.out.println("Test4 (Invalid alg): true");
        }

        // Test 5: Colon-separated fingerprint should be accepted
        String colonExpected1 = colonizeHex(expected1);
        System.out.println("Test5 (Colon-separated fingerprint): " + certificateMatchesHash(cert1, colonExpected1, "SHA-256"));
    }
}