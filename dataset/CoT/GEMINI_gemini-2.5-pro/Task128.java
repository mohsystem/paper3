import java.security.SecureRandom;
import java.security.NoSuchAlgorithmException;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

public class Task128 {

    private static final String CHAR_SET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    private static final SecureRandom secureRandom;

    static {
        try {
            // Using "NativePRNGNonBlocking" on Linux/macOS or "Windows-PRNG" on Windows
            // can be more performant and is still secure. Default is often sufficient.
            secureRandom = SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            // This is highly unlikely to happen on standard systems.
            // Fallback to the default constructor.
            throw new RuntimeException("Could not initialize strong SecureRandom", e);
        }
    }

    /**
     * Generates a cryptographically secure random integer within a specified range.
     * @param min The minimum value (inclusive).
     * @param max The maximum value (inclusive).
     * @return A random integer between min and max.
     */
    public int generateRandomInt(int min, int max) {
        if (min >= max) {
            throw new IllegalArgumentException("max must be greater than min");
        }
        // nextInt(bound) generates a value between 0 (inclusive) and bound (exclusive).
        // So we need to calculate the range size and add the min value.
        return secureRandom.nextInt((max - min) + 1) + min;
    }

    /**
     * Generates a cryptographically secure random alphanumeric token of a specified length.
     * @param length The desired length of the token.
     * @return A random alphanumeric token.
     */
    public String generateRandomToken(int length) {
        if (length <= 0) {
            throw new IllegalArgumentException("Token length must be positive.");
        }

        StringBuilder sb = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            int randomIndex = secureRandom.nextInt(CHAR_SET.length());
            sb.append(CHAR_SET.charAt(randomIndex));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        Task128 generator = new Task128();

        System.out.println("Java Random Generation Test Cases:");

        // Test Case 1
        System.out.println("1. Random integer between 1 and 100: " + generator.generateRandomInt(1, 100));

        // Test Case 2
        System.out.println("2. Random integer between -50 and 50: " + generator.generateRandomInt(-50, 50));

        // Test Case 3
        System.out.println("3. Random token of length 8: " + generator.generateRandomToken(8));

        // Test Case 4
        System.out.println("4. Random token of length 16: " + generator.generateRandomToken(16));

        // Test Case 5
        System.out.println("5. Random integer between 1000 and 5000: " + generator.generateRandomInt(1000, 5000));
    }
}