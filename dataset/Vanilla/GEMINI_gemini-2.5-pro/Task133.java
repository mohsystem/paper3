import java.util.HashMap;
import java.util.Map;

public class Task133 {

    // Simulates a user database
    private static Map<String, String> userDatabase = new HashMap<>();

    /**
     * Resets the password for a given user if the user exists.
     *
     * @param username    The username of the user whose password is to be reset.
     * @param newPassword The new password to set.
     * @return true if the password was successfully reset, false otherwise (e.g., user not found).
     */
    public static boolean resetPassword(String username, String newPassword) {
        if (username == null || newPassword == null || username.isEmpty()) {
            return false;
        }

        if (userDatabase.containsKey(username)) {
            userDatabase.put(username, newPassword);
            return true;
        }
        return false;
    }

    public static void main(String[] args) {
        // Initialize the database with some users
        userDatabase.put("alice", "password123");
        userDatabase.put("bob", "bobspass");
        userDatabase.put("charlie", "securepass");

        System.out.println("Initial Database: " + userDatabase);
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Successful password reset for an existing user
        boolean result1 = resetPassword("alice", "newAlicePass!@#");
        System.out.println("1. Resetting 'alice' password. Success: " + result1);
        System.out.println("   Database after: " + userDatabase);

        // Test Case 2: Attempt to reset password for a non-existent user
        boolean result2 = resetPassword("dave", "davespass");
        System.out.println("\n2. Resetting 'dave' (non-existent) password. Success: " + result2);
        System.out.println("   Database after: " + userDatabase);

        // Test Case 3: Successful reset for another existing user
        boolean result3 = resetPassword("bob", "betterP@ssw0rd");
        System.out.println("\n3. Resetting 'bob' password. Success: " + result3);
        System.out.println("   Database after: " + userDatabase);

        // Test Case 4: Resetting the same user's password again
        boolean result4 = resetPassword("alice", "evenNewerPass456");
        System.out.println("\n4. Resetting 'alice' password again. Success: " + result4);
        System.out.println("   Database after: " + userDatabase);

        // Test Case 5: Attempt to reset password with an empty username
        boolean result5 = resetPassword("", "somepassword");
        System.out.println("\n5. Resetting password for empty username. Success: " + result5);
        System.out.println("   Database after: " + userDatabase);
    }
}