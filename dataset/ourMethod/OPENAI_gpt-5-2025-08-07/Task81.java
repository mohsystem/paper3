import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.Base64;
import java.util.Locale;

public class Task81 {
    // Compute SHA-256 fingerprint of a certificate (DER or PEM input).
    public static String fingerprintHex(byte[] certInput, String algorithm) {
        if (certInput == null || certInput.length == 0) {
            throw new IllegalArgumentException("Empty certificate input");
        }
        String alg = (algorithm == null) ? "SHA-256" : algorithm.trim().toUpperCase(Locale.ROOT);
        if (!"SHA-256".equals(alg)) {
            throw new IllegalArgumentException("Only SHA-256 is supported");
        }
        byte[] der = decodePemIfNeeded(certInput);
        byte[] digest = hash(der, "SHA-256");
        return toHex(digest);
    }

    // Returns true if the certificate's fingerprint matches the known hash in constant time.
    public static boolean matchesKnownHash(byte[] certInput, String knownHash, String algorithm) {
        if (certInput == null || knownHash == null) return false;
        String alg = (algorithm == null) ? "SHA-256" : algorithm.trim().toUpperCase(Locale.ROOT);
        if (!"SHA-256".equals(alg)) return false;
        byte[] der = decodePemIfNeeded(certInput);
        byte[] actual = hash(der, "SHA-256");
        byte[] expected = parseHexFlexible(knownHash);
        if (expected == null || expected.length == 0) return false;
        if (expected.length != actual.length) return false;
        return MessageDigest.isEqual(actual, expected);
    }

    private static byte[] decodePemIfNeeded(byte[] input) {
        String s = new String(input, StandardCharsets.UTF_8);
        String begin = "-----BEGIN CERTIFICATE-----";
        String end = "-----END CERTIFICATE-----";
        if (s.contains(begin) && s.contains(end)) {
            int start = s.indexOf(begin) + begin.length();
            int stop = s.indexOf(end, start);
            if (stop <= start) throw new IllegalArgumentException("Invalid PEM format");
            String b64 = s.substring(start, stop).replaceAll("\\s", "");
            try {
                return Base64.getDecoder().decode(b64);
            } catch (IllegalArgumentException e) {
                throw new IllegalArgumentException("Invalid Base64 in PEM");
            }
        }
        return input;
    }

    private static byte[] hash(byte[] data, String alg) {
        try {
            MessageDigest md = MessageDigest.getInstance(alg);
            return md.digest(data);
        } catch (Exception e) {
            throw new IllegalStateException("Hashing failed");
        }
    }

    private static String toHex(byte[] data) {
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) {
            sb.append(Character.forDigit((b >>> 4) & 0xF, 16));
            sb.append(Character.forDigit(b & 0xF, 16));
        }
        return sb.toString();
    }

    private static byte[] parseHexFlexible(String s) {
        if (s == null) return null;
        String str = s.trim();
        StringBuilder hexOnly = new StringBuilder(str.length());
        for (int i = 0; i < str.length(); i++) {
            char c = str.charAt(i);
            boolean isHex = (c >= '0' && c <= '9') ||
                            (c >= 'a' && c <= 'f') ||
                            (c >= 'A' && c <= 'F');
            boolean isSep = c == ':' || c == ' ' || c == '-' || c == '\t' || c == '\n' || c == '\r';
            if (isHex) {
                hexOnly.append(c);
            } else if (isSep) {
                // skip
            } else {
                return null; // invalid character
            }
        }
        if (hexOnly.length() == 0 || (hexOnly.length() % 2) != 0) return null;
        int len = hexOnly.length() / 2;
        byte[] out = new byte[len];
        for (int i = 0; i < len; i++) {
            int hi = Character.digit(hexOnly.charAt(2 * i), 16);
            int lo = Character.digit(hexOnly.charAt(2 * i + 1), 16);
            if (hi < 0 || lo < 0) return null;
            out[i] = (byte) ((hi << 4) + lo);
        }
        return out;
    }

    // Helper to format colon-separated uppercase fingerprint (for tests)
    private static String toColonUpper(String hexLower) {
        String h = hexLower.toUpperCase(Locale.ROOT);
        StringBuilder sb = new StringBuilder(h.length() + h.length() / 2);
        for (int i = 0; i < h.length(); i++) {
            if (i > 0 && (i % 2) == 0) sb.append(':');
            sb.append(h.charAt(i));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        // Test data
        byte[] cert1 = "DER-CERT-ONE-123".getBytes(StandardCharsets.UTF_8);
        byte[] cert2 = "DER-CERT-TWO-456".getBytes(StandardCharsets.UTF_8);
        byte[] cert3 = "DIFFERENT-CERT-XYZ".getBytes(StandardCharsets.UTF_8);

        // Compute fingerprints for testing
        String fp1 = fingerprintHex(cert1, "SHA-256");
        String fp2 = fingerprintHex(cert2, "SHA-256");
        String fp3 = fingerprintHex(cert3, "SHA-256");

        // Build PEM for cert2
        String pem2 = "-----BEGIN CERTIFICATE-----\n" +
                Base64.getMimeEncoder(64, "\n".getBytes(StandardCharsets.UTF_8))
                        .encodeToString(cert2) +
                "\n-----END CERTIFICATE-----\n";

        // Test cases (5)
        boolean t1 = matchesKnownHash(cert1, fp1, "SHA-256"); // exact match
        boolean t2 = matchesKnownHash(cert1, toColonUpper(fp1), "SHA-256"); // colon/upper match
        boolean t3 = matchesKnownHash(cert1, fp2, "SHA-256"); // mismatch
        boolean t4 = matchesKnownHash(pem2.getBytes(StandardCharsets.UTF_8), fp2, "SHA-256"); // PEM match
        boolean t5 = matchesKnownHash(cert3, "ZZ:11:GG", "SHA-256"); // invalid known hash -> false

        System.out.println("Test1: " + t1);
        System.out.println("Test2: " + t2);
        System.out.println("Test3: " + t3);
        System.out.println("Test4: " + t4);
        System.out.println("Test5: " + t5);
    }
}