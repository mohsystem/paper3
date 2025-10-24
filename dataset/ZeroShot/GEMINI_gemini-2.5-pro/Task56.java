import java.security.SecureRandom;
import java.util.Base64;

public class Task56 {

    /**
     * Generates a secure random token using a cryptographically strong
     * pseudo-random number generator (CSPRNG).
     *
     * @param lengthInBytes The number of random bytes to generate for the token.
     *                      A common length is 32 bytes for high security.
     * @return A URL-safe Base64 encoded string representing the token.
     */
    public static String generateToken(int lengthInBytes) {
        if (lengthInBytes <= 0) {
            // It's not meaningful to generate a token of zero or negative length.
            return "";
        }
        SecureRandom secureRandom = new SecureRandom();
        byte[] tokenBytes = new byte[lengthInBytes];
        secureRandom.nextBytes(tokenBytes);
        // Using URL-safe Base64 encoding is a common practice for tokens.
        return Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);
    }

    public static void main(String[] args) {
        System.out.println("Java: Generating 5 secure tokens (32 bytes of randomness each)");
        for (int i = 0; i < 5; i++) {
            String token = generateToken(32);
            System.out.println("Test Case " + (i + 1) + ": " + token);
        }
    }
}