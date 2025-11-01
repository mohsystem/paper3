public class Lse39 {

    /**
     * Checks if the provided username and password are correct.
     * In a real-world application, this method would check credentials against a database
     * and use hashed passwords, not plaintext.
     *
     * @param username The username to check.
     * @param password The password to check.
     * @return A string indicating success or failure.
     */
    public static String login(String username, String password) {
        // NOTE: Hardcoding credentials is insecure and only for demonstration.
        final String CORRECT_USERNAME = "admin";
        final String CORRECT_PASSWORD = "password123";

        // Use .equals() for safe string comparison in Java.
        // A real application should use a constant-time comparison to prevent timing attacks.
        if (CORRECT_USERNAME.equals(username) && CORRECT_PASSWORD.equals(password)) {
            return "Login successful! Redirecting to the session page.";
        } else {
            return "Incorrect username or password.";
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Login System Test Cases ---");

        // Test Case 1: Correct credentials
        System.out.println("\nTest Case 1 (Correct):");
        System.out.println("Input: user='admin', pass='password123'");
        System.out.println("Output: " + login("admin", "password123"));

        // Test Case 2: Incorrect password
        System.out.println("\nTest Case 2 (Incorrect Password):");
        System.out.println("Input: user='admin', pass='wrongpassword'");
        System.out.println("Output: " + login("admin", "wrongpassword"));

        // Test Case 3: Incorrect username
        System.out.println("\nTest Case 3 (Incorrect Username):");
        System.out.println("Input: user='user', pass='password123'");
        System.out.println("Output: " + login("user", "password123"));

        // Test Case 4: Both incorrect
        System.out.println("\nTest Case 4 (Both Incorrect):");
        System.out.println("Input: user='user', pass='wrongpassword'");
        System.out.println("Output: " + login("user", "wrongpassword"));

        // Test Case 5: Case-sensitive check
        System.out.println("\nTest Case 5 (Case-Sensitive):");
        System.out.println("Input: user='Admin', pass='password123'");
        System.out.println("Output: " + login("Admin", "password123"));
        
        System.out.println("\n--- End of Test Cases ---");
    }
}