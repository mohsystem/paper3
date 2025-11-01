import java.util.HashMap;
import java.util.Map;

class Lse137 {

    // In a real application, this data would come from a secure database.
    // Passwords should be securely hashed (e.g., using bcrypt), not stored in plain text.
    private static final Map<String, UserData> userDatabase = new HashMap<>();

    static {
        // Populate the mock database
        userDatabase.put("admin", new UserData("pass123", "adminsession123", "moderator"));
        userDatabase.put("user", new UserData("password", "usersession456", "user"));
    }

    static class UserData {
        String password; // In a real app, this would be a hash.
        String session;
        String role;

        UserData(String password, String session, String role) {
            this.password = password;
            this.session = session;
            this.role = role;
        }
    }

    /**
     * Authenticates a user and checks if they have the 'moderator' role.
     * 
     * SECURITY NOTE: In a real-world scenario:
     * 1. Passwords must be compared using a secure hash comparison function to prevent timing attacks.
     * 2. This function should consult a database or a secure authentication service.
     * 3. Session tokens should be long, random, and managed securely.
     *
     * @param username The user's name.
     * @param password The user's plain-text password.
     * @param session  The user's session token.
     * @return true if the user is authenticated and is a moderator, false otherwise.
     */
    public static boolean auth_mod(String username, String password, String session) {
        if (username == null || password == null || session == null) {
            return false;
        }

        UserData userData = userDatabase.get(username);

        if (userData != null) {
            // NOTE: Plain text password comparison is insecure. Use a constant-time hash comparison.
            boolean isPasswordCorrect = userData.password.equals(password);
            boolean isSessionCorrect = userData.session.equals(session);
            boolean isModerator = "moderator".equals(userData.role);

            return isPasswordCorrect && isSessionCorrect && isModerator;
        }
        
        return false;
    }

    /**
     * Checks if a user is a moderator based on parameters.
     * 
     * SECURITY NOTE: Getting credentials from a URL is extremely insecure (CWE-598).
     * Sensitive data like passwords and session tokens should be sent in the body
     * of a POST request over HTTPS.
     *
     * @param username The user's name.
     * @param password The user's plain-text password.
     * @param session  The user's session token.
     * @return true if the user is a moderator, false otherwise.
     */
    public static boolean check_mod(String username, String password, String session) {
        return auth_mod(username, password, session);
    }

    public static void main(String[] args) {
        // Test Cases
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Valid moderator credentials
        boolean test1 = check_mod("admin", "pass123", "adminsession123");
        System.out.println("Test Case 1 (Valid Moderator): " + (test1 ? "Passed" : "Failed"));

        // Test Case 2: Valid user but not a moderator
        boolean test2 = check_mod("user", "password", "usersession456");
        System.out.println("Test Case 2 (Not a Moderator): " + (!test2 ? "Passed" : "Failed"));

        // Test Case 3: Correct username, wrong password
        boolean test3 = check_mod("admin", "wrongpassword", "adminsession123");
        System.out.println("Test Case 3 (Wrong Password): " + (!test3 ? "Passed" : "Failed"));

        // Test Case 4: Correct credentials, wrong session
        boolean test4 = check_mod("admin", "pass123", "wrongsession");
        System.out.println("Test Case 4 (Wrong Session): " + (!test4 ? "Passed" : "Failed"));

        // Test Case 5: Non-existent user
        boolean test5 = check_mod("unknown", "password", "somesession");
        System.out.println("Test Case 5 (Non-existent User): " + (!test5 ? "Passed" : "Failed"));
    }
}