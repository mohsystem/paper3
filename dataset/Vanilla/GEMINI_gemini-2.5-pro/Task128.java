import java.util.Random;

public class Task128 {

    private static final String TOKEN_CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    private static final Random random = new Random();

    /**
     * Generates a random integer within a specified range (inclusive).
     *
     * @param min The minimum value of the range.
     * @param max The maximum value of the range.
     * @return A random integer between min and max.
     */
    public static int generateRandomNumber(int min, int max) {
        if (min > max) {
            throw new IllegalArgumentException("min must not be greater than max");
        }
        return random.nextInt((max - min) + 1) + min;
    }

    /**
     * Generates a random alphanumeric token of a specified length.
     *
     * @param length The desired length of the token.
     * @return A random alphanumeric token.
     */
    public static String generateRandomToken(int length) {
        if (length <= 0) {
            return "";
        }
        StringBuilder token = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            int randomIndex = random.nextInt(TOKEN_CHARACTERS.length());
            token.append(TOKEN_CHARACTERS.charAt(randomIndex));
        }
        return token.toString();
    }

    public static void main(String[] args) {
        System.out.println("--- Java Random Number and Token Generation ---");

        // Test cases for random number generation
        System.out.println("\n--- Random Numbers ---");
        System.out.println("1. Random number between 1 and 100: " + generateRandomNumber(1, 100));
        System.out.println("2. Random number between -50 and 50: " + generateRandomNumber(-50, 50));
        System.out.println("3. Random number between 1000 and 2000: " + generateRandomNumber(1000, 2000));
        System.out.println("4. Random number between 0 and 1: " + generateRandomNumber(0, 1));
        System.out.println("5. Random number between 5 and 5: " + generateRandomNumber(5, 5));

        // Test cases for random token generation
        System.out.println("\n--- Random Tokens ---");
        System.out.println("1. Token of length 8: " + generateRandomToken(8));
        System.out.println("2. Token of length 12: " + generateRandomToken(12));
        System.out.println("3. Token of length 16: " + generateRandomToken(16));
        System.out.println("4. Token of length 1: " + generateRandomToken(1));
        System.out.println("5. Token of length 32: " + generateRandomToken(32));
    }
}