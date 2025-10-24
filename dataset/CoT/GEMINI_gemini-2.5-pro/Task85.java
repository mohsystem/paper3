import java.security.SecureRandom;

public class Task85 {

    /**
     * Generates an unpredictable random string of a specified length using only ASCII letters.
     * This method uses java.security.SecureRandom, which is a cryptographically strong
     * random number generator (CSPRNG).
     *
     * @param length The desired length of the string.
     * @return A random string of the specified length, or an empty string if length is <= 0.
     */
    public static String generateRandomString(int length) {
        if (length <= 0) {
            return "";
        }

        final String CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        SecureRandom random = new SecureRandom();
        StringBuilder sb = new StringBuilder(length);

        for (int i = 0; i < length; i++) {
            int randomIndex = random.nextInt(CHARS.length());
            sb.append(CHARS.charAt(randomIndex));
        }

        return sb.toString();
    }

    public static void main(String[] args) {
        System.out.println("Java Test Cases:");

        // Test Case 1: Standard length
        int len1 = 16;
        String randomStr1 = generateRandomString(len1);
        System.out.println("1. Length " + len1 + ": " + randomStr1 + " (Actual length: " + randomStr1.length() + ")");

        // Test Case 2: Short length
        int len2 = 5;
        String randomStr2 = generateRandomString(len2);
        System.out.println("2. Length " + len2 + ": " + randomStr2 + " (Actual length: " + randomStr2.length() + ")");

        // Test Case 3: Long length
        int len3 = 64;
        String randomStr3 = generateRandomString(len3);
        System.out.println("3. Length " + len3 + ": " + randomStr3 + " (Actual length: " + randomStr3.length() + ")");
        
        // Test Case 4: Zero length
        int len4 = 0;
        String randomStr4 = generateRandomString(len4);
        System.out.println("4. Length " + len4 + ": \"" + randomStr4 + "\" (Actual length: " + randomStr4.length() + ")");

        // Test Case 5: Negative length
        int len5 = -10;
        String randomStr5 = generateRandomString(len5);
        System.out.println("5. Length " + len5 + ": \"" + randomStr5 + "\" (Actual length: " + randomStr5.length() + ")");
    }
}