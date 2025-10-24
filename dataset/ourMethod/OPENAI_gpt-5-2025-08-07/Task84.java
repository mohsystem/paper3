import java.security.SecureRandom;
import java.util.Base64;

public final class Task84 {
    private static final int MIN_BYTES = 16;
    private static final int MAX_BYTES = 64;

    public static String generateSessionId(int numBytes) {
        if (numBytes < MIN_BYTES || numBytes > MAX_BYTES) {
            throw new IllegalArgumentException("numBytes must be between " + MIN_BYTES + " and " + MAX_BYTES);
        }
        final byte[] randomBytes = new byte[numBytes];
        SecureRandom sr = new SecureRandom();
        sr.nextBytes(randomBytes);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);
    }

    public static void main(String[] args) {
        int[] tests = new int[] {16, 24, 32, 48, 64};
        for (int t : tests) {
            try {
                String id = generateSessionId(t);
                System.out.println(id);
            } catch (IllegalArgumentException e) {
                System.out.println("Error: " + e.getMessage());
            }
        }
    }
}