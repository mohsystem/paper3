import java.security.SecureRandom;

public class Task85 {

    private static final String ALPHABET = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    private static final SecureRandom RANDOM = new SecureRandom();

    /**
     * Generates a cryptographically secure random string of a given length.
     * The string will consist only of ASCII letters (a-z, A-Z).
     *
     * @param length The desired length of the string. Must be non-negative.
     * @return A random string of the specified length, or an empty string if length is <= 0.
     */
    public static String generateRandomString(int length) {
        if (length <= 0) {
            return "";
        }

        StringBuilder sb = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            int randomIndex = RANDOM.nextInt(ALPHABET.length());
            sb.append(ALPHABET.charAt(randomIndex));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Length 10
        String s1 = generateRandomString(10);
        System.out.println("Test 1 (length 10): " + s1 + " (length: " + s1.length() + ")");

        // Test Case 2: Length 0
        String s2 = generateRandomString(0);
        System.out.println("Test 2 (length 0): \"" + s2 + "\" (length: " + s2.length() + ")");

        // Test Case 3: Length 1
        String s3 = generateRandomString(1);
        System.out.println("Test 3 (length 1): " + s3 + " (length: " + s3.length() + ")");

        // Test Case 4: Length 32
        String s4 = generateRandomString(32);
        System.out.println("Test 4 (length 32): " + s4 + " (length: " + s4.length() + ")");

        // Test Case 5: Negative length
        String s5 = generateRandomString(-5);
        System.out.println("Test 5 (length -5): \"" + s5 + "\" (length: " + s5.length() + ")");
    }
}