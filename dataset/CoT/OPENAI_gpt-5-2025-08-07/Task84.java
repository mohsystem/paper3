// Step 1-5: Secure session ID generator in Java
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

public final class Task84 {
    // Thread-safe set to ensure uniqueness within this runtime
    private static final Set<String> ISSUED_IDS = Collections.synchronizedSet(new HashSet<>());
    private static final SecureRandom RNG = createSecureRandom();
    private static final int MIN_BYTES = 16;
    private static final int MAX_BYTES = 1024;
    private static final int MAX_ATTEMPTS = 1000;

    private static SecureRandom createSecureRandom() {
        try {
            return SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            // Fallback to default SecureRandom if strong algorithm not available
            return new SecureRandom();
        }
    }

    // Generates a unique, URL-safe Base64-encoded session ID without padding
    public static String generateSessionId(int numBytes) {
        if (numBytes < MIN_BYTES || numBytes > MAX_BYTES) {
            throw new IllegalArgumentException("numBytes must be between " + MIN_BYTES + " and " + MAX_BYTES);
        }
        byte[] buf = new byte[numBytes];
        for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
            RNG.nextBytes(buf);
            String id = Base64.getUrlEncoder().withoutPadding().encodeToString(buf);
            if (ISSUED_IDS.add(id)) {
                return id;
            }
        }
        throw new IllegalStateException("Unable to generate a unique session ID after " + MAX_ATTEMPTS + " attempts");
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(generateSessionId(16));
        System.out.println(generateSessionId(24));
        System.out.println(generateSessionId(32));
        System.out.println(generateSessionId(48));
        System.out.println(generateSessionId(64));
    }
}