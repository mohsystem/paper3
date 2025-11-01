public class Lse136 {

    /**
     * NOTE: Storing credentials in plaintext is a major security risk.
     * In a real application, use a strong, salted hashing algorithm (e.g., Argon2, bcrypt)
     * to store password hashes and compare against the hash of the user-provided password.
     * These are hardcoded for demonstration purposes only.
     */
    private static final String CORRECT_USERNAME = "admin";
    private static final String CORRECT_PASSWORD = "password123";

    /**
     * Performs a constant-time string comparison to mitigate timing attacks.
     *
     * @param s1 The first string.
     * @param s2 The second string.
     * @return true if the strings are equal, false otherwise.
     */
    private static boolean secureEquals(String s1, String s2) {
        if (s1 == null || s2 == null) {
            return s1 == s2;
        }

        byte[] bytes1 = s1.getBytes();
        byte[] bytes2 = s2.getBytes();

        if (bytes1.length != bytes2.length) {
            return false;
        }

        int diff = 0;
        for (int i = 0; i < bytes1.length; i++) {
            diff |= bytes1[i] ^ bytes2[i];
        }
        return diff == 0;
    }

    /**
     * Simulates checking a username and password from a URL.
     * Returns "true" if credentials are correct, "false" otherwise.
     *
     * @param username The username to check.
     * @param password The password to check.
     * @return The string "true" on success, "false" on failure.
     */
    public static String check_mod(String username, String password) {
        // Usernames are often not secret, so a standard comparison is acceptable.
        boolean isUsernameCorrect = CORRECT_USERNAME.equals(username);
        
        // Passwords must be compared in constant time to prevent timing attacks.
        boolean isPasswordCorrect = secureEquals(CORRECT_PASSWORD, password);

        if (isUsernameCorrect && isPasswordCorrect) {
            return "true";
        } else {
            return "false";
        }
    }

    public static void main(String[] args) {
        // Test cases
        String[][] testCases = {
            {"admin", "password123"},      // 1. Correct credentials
            {"admin", "wrongpassword"},     // 2. Correct user, wrong password
            {"user", "password123"},      // 3. Wrong user, correct password
            {"user", "wrongpassword"},      // 4. Wrong credentials
            {"", ""}                       // 5. Empty credentials
        };

        System.out.println("Running test cases...");
        for (int i = 0; i < testCases.length; i++) {
            String username = testCases[i][0];
            String password = testCases[i][1];
            String result = check_mod(username, password);
            System.out.printf("Test Case %d:\n", i + 1);
            System.out.printf("  Username: \"%s\"\n", username);
            System.out.printf("  Password: \"%s\"\n", password);
            System.out.printf("  Result: %s\n\n", result);
        }
    }
}