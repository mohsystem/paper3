// Chain-of-Through process in code generation:
// 1) Problem understanding: Generate a cryptographically strong, URL-safe token for user authentication.
// 2) Security requirements: Use cryptographically secure RNG, sufficient entropy (>=128 bits), URL-safe encoding, avoid predictable state.
// 3) Secure coding generation: Validate input size, use SecureRandom, Base64 URL-safe without padding, avoid leaking sensitive state.
// 4) Code review: Checked for predictable RNG, weak encodings, insufficient entropy, improper error handling.
// 5) Secure code output: Final code mitigates risks by using strong RNG, proper encoding, and sanity checks.

import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;

public class Task56 {

    // Generate a URL-safe token using cryptographically secure random bytes.
    // numBytes must be between 16 and 64 (inclusive) to ensure sufficient entropy and reasonable size.
    public static String generateAuthToken(int numBytes) {
        if (numBytes < 16 || numBytes > 64) {
            throw new IllegalArgumentException("numBytes must be between 16 and 64");
        }

        final SecureRandom sr = secureRandom();
        byte[] bytes = new byte[numBytes];
        sr.nextBytes(bytes);

        // URL-safe Base64 without padding to avoid issues in URLs/cookies.
        return Base64.getUrlEncoder().withoutPadding().encodeToString(bytes);
    }

    // Prefer a strong instance; fallback to default if unavailable.
    private static SecureRandom secureRandom() {
        try {
            return SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            // Fallback is still cryptographically secure on modern JVMs.
            return new SecureRandom();
        }
    }

    // 5 test cases
    public static void main(String[] args) {
        int[] tests = {16, 24, 32, 48, 64};
        for (int n : tests) {
            String token = generateAuthToken(n);
            System.out.println("Bytes=" + n + " Token=" + token + " Length=" + token.length());
        }
    }
}