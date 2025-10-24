import java.security.SecureRandom;
import java.security.NoSuchAlgorithmException;
import java.util.Formatter;

public class Task128 {

    private static final SecureRandom secureRandom;

    static {
        try {
            // Use a strong CSPRNG. "NativePRNG" is often a good choice on Linux/macOS.
            // On Windows it will default to "Windows-PRNG".
            secureRandom = SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            // Fallback to the default PRNG if the strong one is not available.
            // The default is still a cryptographically secure PRNG.
            throw new RuntimeException("Failed to initialize SecureRandom", e);
        }
    }

    /**
     * Generates a cryptographically secure random integer within a specified range.
     *
     * @param min The minimum value of the range (inclusive).
     * @param max The maximum value of the range (inclusive).
     * @return A random integer within the specified range.
     * @throws IllegalArgumentException if min is greater than or equal to max.
     */
    public static int generateRandomInt(int min, int max) {
        if (min >= max) {
            throw new IllegalArgumentException("Max must be greater than min.");
        }
        // nextInt(bound) generates a value between 0 (inclusive) and bound (exclusive).
        // To get a value in [min, max], we need a bound of (max - min + 1).
        return secureRandom.nextInt(max - min + 1) + min;
    }

    /**
     * Generates a cryptographically secure random token as a hexadecimal string.
     *
     * @param numBytes The number of random bytes to generate. The resulting hex string will be 2 * numBytes long.
     * @return A random token encoded as a hexadecimal string.
     * @throws IllegalArgumentException if numBytes is not positive.
     */
    public static String generateRandomTokenHex(int numBytes) {
        if (numBytes <= 0) {
            throw new IllegalArgumentException("Number of bytes must be positive.");
        }
        byte[] randomBytes = new byte[numBytes];
        secureRandom.nextBytes(randomBytes);
        
        // Using a Formatter is an efficient way to convert bytes to a hex string.
        StringBuilder sb = new StringBuilder(numBytes * 2);
        try (Formatter formatter = new Formatter(sb)) {
            for (byte b : randomBytes) {
                formatter.format("%02x", b);
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        System.out.println("--- Testing Random Number and Token Generation ---");

        // Test Case 1: Standard integer range
        System.out.println("Test Case 1: Random int between 1 and 100");
        int randomInt1 = generateRandomInt(1, 100);
        System.out.println("Generated Number: " + randomInt1);
        System.out.println();

        // Test Case 2: Negative integer range
        System.out.println("Test Case 2: Random int between -50 and -10");
        int randomInt2 = generateRandomInt(-50, -10);
        System.out.println("Generated Number: " + randomInt2);
        System.out.println();

        // Test Case 3: 16-byte (128-bit) random token
        System.out.println("Test Case 3: 16-byte random token (32 hex characters)");
        String token1 = generateRandomTokenHex(16);
        System.out.println("Generated Token: " + token1);
        System.out.println();

        // Test Case 4: 32-byte (256-bit) random token
        System.out.println("Test Case 4: 32-byte random token (64 hex characters)");
        String token2 = generateRandomTokenHex(32);
        System.out.println("Generated Token: " + token2);
        System.out.println();
        
        // Test Case 5: Invalid range for integer generation
        System.out.println("Test Case 5: Invalid range (100, 1)");
        try {
            generateRandomInt(100, 1);
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
    }
}