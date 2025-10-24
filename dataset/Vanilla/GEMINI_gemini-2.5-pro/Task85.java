import java.security.SecureRandom;

public class Task85 {

    private static final String CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    private static final SecureRandom RANDOM = new SecureRandom();

    /**
     * Generates an unpredictable random string of a given length.
     * The string consists only of ASCII letters (uppercase and lowercase).
     * @param length The length of the string to generate.
     * @return The randomly generated string.
     */
    public static String generateRandomString(int length) {
        if (length < 0) {
            throw new IllegalArgumentException("Length must be a non-negative number.");
        }
        if (length == 0) {
            return "";
        }

        StringBuilder sb = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            int randomIndex = RANDOM.nextInt(CHARACTERS.length());
            sb.append(CHARACTERS.charAt(randomIndex));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        System.out.println("Java Test Cases:");
        
        int[] testLengths = {10, 15, 8, 0, 25};
        
        for (int len : testLengths) {
            try {
                String randomString = generateRandomString(len);
                System.out.println("Length " + len + ": " + randomString);
            } catch (IllegalArgumentException e) {
                 System.out.println("Error for length " + len + ": " + e.getMessage());
            }
        }
    }
}