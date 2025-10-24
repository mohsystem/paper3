import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;

public class Task56 {

    // Generates a URL-safe base64 token with cryptographically secure randomness.
    // Ensures at least 32 bytes of entropy.
    public static String generateToken(int numBytes) {
        int minBytes = 32;
        int n = Math.max(numBytes, minBytes);
        byte[] randomBytes = new byte[n];
        SecureRandom sr = getSecureRandom();
        sr.nextBytes(randomBytes);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);
    }

    private static SecureRandom getSecureRandom() {
        try {
            // Strong may block on some systems; fallback to default if unavailable
            return SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            return new SecureRandom();
        }
    }

    // Simple test harness with 5 cases
    public static void main(String[] args) {
        int[] testSizes = new int[] {32, 48, 64, 16, 0};
        for (int i = 0; i < testSizes.length; i++) {
            String tok = generateToken(testSizes[i]);
            System.out.println("Java Token " + (i + 1) + " (" + Math.max(testSizes[i], 32) + " bytes): " + tok);
        }
    }
}