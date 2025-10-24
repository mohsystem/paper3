import java.security.SecureRandom;

public class Task128 {

    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    private static final String TOKEN_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    /**
     * Generates a cryptographically secure random integer within a specified range.
     *
     * @param min The minimum value of the range (inclusive).
     * @param max The maximum value of the range (inclusive).
     * @return A secure random integer.
     * @throws IllegalArgumentException if max is less than min.
     */
    public static int generateSecureRandomNumber(int min, int max) {
        if (min > max) {
            throw new IllegalArgumentException("Max must be greater than or equal to Min.");
        }
        // nextInt(bound) generates a value between 0 (inclusive) and bound (exclusive).
        // To get a value in [min, max], we need a range of (max - min + 1).
        return SECURE_RANDOM.nextInt(max - min + 1) + min;
    }

    /**
     * Generates a cryptographically secure random alphanumeric token of a given length.
     *
     * @param length The desired length of the token.
     * @return A secure random token string.
     * @throws IllegalArgumentException if length is not positive.
     */
    public static String generateSecureToken(int length) {
        if (length <= 0) {
             throw new IllegalArgumentException("Token length must be positive.");
        }
        
        StringBuilder tokenBuilder = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            int randomIndex = SECURE_RANDOM.nextInt(TOKEN_CHARS.length());
            tokenBuilder.append(TOKEN_CHARS.charAt(randomIndex));
        }
        return tokenBuilder.toString();
    }

    public static void main(String[] args) {
        System.out.println("--- Secure Random Number Generation ---");
        for (int i = 0; i < 5; i++) {
            int min = i * 10;
            int max = 50 + i * 10;
            try {
                int randomNumber = generateSecureRandomNumber(min, max);
                System.out.printf("Test %d: Random number between %d and %d: %d%n", i + 1, min, max, randomNumber);
            } catch (IllegalArgumentException e) {
                System.err.println("Error generating number: " + e.getMessage());
            }
        }
        
        System.out.println("\n--- Secure Random Token Generation ---");
        for (int i = 0; i < 5; i++) {
            int length = 8 + i * 4;
            try {
                String token = generateSecureToken(length);
                System.out.printf("Test %d: Token of length %d: %s%n", i + 1, length, token);
            } catch (IllegalArgumentException e) {
                System.err.println("Error generating token: " + e.getMessage());
            }
        }
    }
}