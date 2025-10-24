import java.security.SecureRandom;

public class Task85 {

    private static final String ASCII_LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    private static final SecureRandom secureRandom = new SecureRandom();

    /**
     * Generates a cryptographically secure random string of a specified length.
     * The string consists only of ASCII letters (a-z, A-Z).
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
            int randomIndex = secureRandom.nextInt(ASCII_LETTERS.length());
            sb.append(ASCII_LETTERS.charAt(randomIndex));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        System.out.println("Java Test Cases:");
        int[] testLengths = {16, 32, 1, 0, -5};
        for (int len : testLengths) {
            String randomStr = generateRandomString(len);
            System.out.println("Length " + len + " -> \"" + randomStr + "\" (Actual length: " + randomStr.length() + ")");
        }
    }
}