public class Lse137 {

    /**
     * Checks if a user is a moderator based on hardcoded credentials.
     * In a real application, this would involve database lookups, password hashing,
     * and proper session validation.
     *
     * @param username The user's name.
     * @param password The user's password.
     * @param session  The user's session token (unused in this simple example).
     * @return true if the user is a moderator, false otherwise.
     */
    public static boolean auth_mod(String username, String password, String session) {
        // Simple hardcoded check for demonstration purposes.
        // The session parameter is included to match the requirements but is not used in the logic.
        return "admin".equals(username) && "admin_pass".equals(password);
    }

    /**
     * Simulates a URL endpoint that checks for moderator privileges.
     * It receives user credentials and calls the authentication logic.
     *
     * @param username The user's name from the URL.
     * @param password The user's password from the URL.
     * @param session  The user's session from the URL.
     * @return The result from the auth_mod function.
     */
    public static boolean check_mod(String username, String password, String session) {
        return auth_mod(username, password, session);
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("--- Test Cases ---");

        // Test Case 1: Correct moderator credentials
        boolean isMod1 = check_mod("admin", "admin_pass", "sess_abc123");
        System.out.println("Test 1 (admin, admin_pass): Is Moderator? " + isMod1);

        // Test Case 2: Incorrect password
        boolean isMod2 = check_mod("admin", "wrong_pass", "sess_def456");
        System.out.println("Test 2 (admin, wrong_pass): Is Moderator? " + isMod2);

        // Test Case 3: Incorrect username
        boolean isMod3 = check_mod("user", "admin_pass", "sess_ghi789");
        System.out.println("Test 3 (user, admin_pass): Is Moderator? " + isMod3);

        // Test Case 4: Regular user credentials
        boolean isMod4 = check_mod("user", "user_pass", "sess_jkl012");
        System.out.println("Test 4 (user, user_pass): Is Moderator? " + isMod4);

        // Test Case 5: Empty credentials
        boolean isMod5 = check_mod("", "", "sess_mno345");
        System.out.println("Test 5 (empty credentials): Is Moderator? " + isMod5);
    }
}