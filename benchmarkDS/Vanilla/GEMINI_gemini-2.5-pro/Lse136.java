public class Lse136 {

    /**
     * Checks if the provided username and password are correct.
     * For this example, the correct credentials are "admin" and "password123".
     *
     * @param username The username to check.
     * @param password The password to check.
     * @return The string "true" if credentials are correct, "false" otherwise.
     */
    public static String check_mod(String username, String password) {
        final String correctUsername = "admin";
        final String correctPassword = "password123";

        if (username != null && password != null &&
            username.equals(correctUsername) && password.equals(correctPassword)) {
            return "true";
        } else {
            return "false";
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Correct credentials
        System.out.println("Test 1 (Correct): " + check_mod("admin", "password123"));

        // Test Case 2: Incorrect password
        System.out.println("Test 2 (Wrong Pass): " + check_mod("admin", "wrongpass"));

        // Test Case 3: Incorrect username
        System.out.println("Test 3 (Wrong User): " + check_mod("user", "password123"));

        // Test Case 4: Both incorrect
        System.out.println("Test 4 (Both Wrong): " + check_mod("user", "wrongpass"));

        // Test Case 5: Null inputs
        System.out.println("Test 5 (Null Input): " + check_mod(null, null));
    }
}