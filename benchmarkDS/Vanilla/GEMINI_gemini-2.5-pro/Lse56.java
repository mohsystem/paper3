public class Lse56 {

    // Simulate a single user's session data
    private static String currentUserEmail = null;
    private static String currentUserPassword = null;
    private static boolean isLoggedIn = false;

    /**
     * Helper to set up an initial user for testing.
     * @param email The user's email.
     * @param password The user's password.
     */
    public static void registerUser(String email, String password) {
        currentUserEmail = email;
        currentUserPassword = password;
        isLoggedIn = false; // User is registered, not logged in
    }

    /**
     * Logs a user in.
     * @param email The email to log in with.
     * @param password The password for the account.
     * @return true if login is successful, false otherwise.
     */
    public static boolean login(String email, String password) {
        if (currentUserEmail != null && currentUserEmail.equals(email) &&
            currentUserPassword != null && currentUserPassword.equals(password)) {
            isLoggedIn = true;
            return true;
        }
        isLoggedIn = false;
        return false;
    }

    /**
     * Logs the current user out.
     */
    public static void logout() {
        isLoggedIn = false;
    }

    /**
     * Changes the current user's email after validation.
     * @param oldEmail The user's current email for confirmation.
     * @param newEmail The desired new email.
     * @param password The user's password for confirmation.
     * @return A String message indicating success or the specific failure reason.
     */
    public static String changeEmail(String oldEmail, String newEmail, String password) {
        if (!isLoggedIn) {
            return "Error: User is not logged in.";
        }
        if (!currentUserEmail.equals(oldEmail)) {
            return "Error: Old email does not match the current email.";
        }
        if (!currentUserPassword.equals(password)) {
            return "Error: Incorrect password.";
        }
        if (oldEmail.equals(newEmail)) {
            return "Error: New email cannot be the same as the old email.";
        }

        // If all checks pass, change the email
        currentUserEmail = newEmail;
        return "Email changed successfully to " + newEmail;
    }

    public static void main(String[] args) {
        // Initial setup
        String initialEmail = "user@example.com";
        String initialPassword = "password123";
        
        // --- Test Case 1: Successful Email Change ---
        System.out.println("--- Test Case 1: Successful Change ---");
        registerUser(initialEmail, initialPassword);
        System.out.println("User registered: " + initialEmail);
        login(initialEmail, initialPassword);
        System.out.println("Login status: " + isLoggedIn);
        String result1 = changeEmail(initialEmail, "new.user@example.com", initialPassword);
        System.out.println("Change email result: " + result1);
        System.out.println("Current email is now: " + currentUserEmail);
        System.out.println();

        // --- Test Case 2: Failed Change (Incorrect Password) ---
        System.out.println("--- Test Case 2: Incorrect Password ---");
        registerUser(initialEmail, initialPassword); // Reset state
        System.out.println("User registered: " + initialEmail);
        login(initialEmail, initialPassword);
        System.out.println("Login status: " + isLoggedIn);
        String result2 = changeEmail(initialEmail, "another.user@example.com", "wrongpassword");
        System.out.println("Change email result: " + result2);
        System.out.println("Current email is still: " + currentUserEmail);
        System.out.println();

        // --- Test Case 3: Failed Change (Not Logged In) ---
        System.out.println("--- Test Case 3: Not Logged In ---");
        registerUser(initialEmail, initialPassword);
        System.out.println("User registered: " + initialEmail);
        logout(); // Ensure user is logged out
        System.out.println("Login status: " + isLoggedIn);
        String result3 = changeEmail(initialEmail, "new.user@example.com", initialPassword);
        System.out.println("Change email result: " + result3);
        System.out.println();

        // --- Test Case 4: Failed Change (New Email is Same as Old) ---
        System.out.println("--- Test Case 4: New Email is Same as Old ---");
        registerUser(initialEmail, initialPassword);
        System.out.println("User registered: " + initialEmail);
        login(initialEmail, initialPassword);
        System.out.println("Login status: " + isLoggedIn);
        String result4 = changeEmail(initialEmail, initialEmail, initialPassword);
        System.out.println("Change email result: " + result4);
        System.out.println();

        // --- Test Case 5: Failed Change (Incorrect Old Email Provided) ---
        System.out.println("--- Test Case 5: Incorrect Old Email Provided ---");
        registerUser(initialEmail, initialPassword);
        System.out.println("User registered: " + initialEmail);
        login(initialEmail, initialPassword);
        System.out.println("Login status: " + isLoggedIn);
        String result5 = changeEmail("wrong.old@example.com", "new.user@example.com", initialPassword);
        System.out.println("Change email result: " + result5);
        System.out.println();
    }
}