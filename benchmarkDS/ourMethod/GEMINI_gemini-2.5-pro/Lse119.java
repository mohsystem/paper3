import java.security.SecureRandom;
import java.util.Arrays;

public class Lse119 {

    /**
     * Generates three cryptographically secure random floats and returns them as strings.
     * This method uses SecureRandom, which is a cryptographically strong random number generator (CSPRNG).
     *
     * @return An array of three strings, each representing a random float.
     */
    public static String[] generateRandomFloatStrings() {
        SecureRandom secureRandom = new SecureRandom();
        String[] result = new String[3];
        for (int i = 0; i < 3; i++) {
            // Generates a float value between 0.0 (inclusive) and 1.0 (exclusive).
            float randomFloat = secureRandom.nextFloat();
            result[i] = String.valueOf(randomFloat);
        }
        return result;
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases...");
        for (int i = 0; i < 5; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            try {
                String[] randomStrings = generateRandomFloatStrings();
                System.out.println("  String 1: " + randomStrings[0]);
                System.out.println("  String 2: " + randomStrings[1]);
                System.out.println("  String 3: " + randomStrings[2]);
            } catch (Exception e) {
                System.err.println("An error occurred: " + e.getMessage());
            }
            System.out.println();
        }
    }
}