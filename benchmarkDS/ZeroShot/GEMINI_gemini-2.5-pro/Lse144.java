import java.util.regex.Pattern;
import java.util.Set;
import java.util.HashSet;
import java.util.Arrays;

public class Lse144 {

    // Mock database of subscribed emails. In a real application, this would be a database call.
    private static final Set<String> emailDatabase = new HashSet<>(Arrays.asList(
        "test@example.com",
        "user1@test.com",
        "fail@example.com",
        "subscriber@newsletter.org"
    ));

    // A simple email validation regex.
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$"
    );

    /**
     * Simulates checking if an email exists in the database.
     * @param email The email to check.
     * @return true if the email exists, false otherwise.
     */
    private static boolean emailExists(String email) {
        // SECURITY: In a real application, use a parameterized query or a prepared
        // statement to prevent SQL injection when querying the database.
        // e.g., "SELECT COUNT(*) FROM users WHERE email = ?"
        return emailDatabase.contains(email);
    }

    /**
     * Simulates running an external script to unsubscribe an email.
     * @param email The email to unsubscribe.
     * @return 0 on success, a non-zero value on failure.
     */
    private static int runUnsubscribeScript(String email) {
        // SECURITY: Never build a command string by concatenating user input.
        // This prevents command injection vulnerabilities. Pass the email as a
        // separate, sanitized argument to the script.
        // e.g., new ProcessBuilder("path/to/script.sh", email).start();

        System.out.println("   -> Running unsubscribe script for: " + email);

        // Mock logic: script fails if the email contains "fail".
        if (email.contains("fail")) {
            System.out.println("   -> Script failed.");
            return -1; // Non-zero indicates failure
        }
        
        System.out.println("   -> Script succeeded.");
        return 0; // 0 indicates success
    }

    /**
     * Checks if an email exists and unsubscribes it.
     * @param email The email address to process.
     * @return 1 if the email existed and the unsubscribe script succeeded.
     *         0 in all other cases (email not found, invalid email, or script failure).
     */
    public static int unsubscribeEmail(String email) {
        // 1. Input Validation
        if (email == null || !EMAIL_PATTERN.matcher(email).matches()) {
            System.out.println("Input '" + email + "' is invalid.");
            return 0;
        }

        // 2. Check if email exists in the database.
        if (!emailExists(email)) {
            System.out.println("Email '" + email + "' not found in database.");
            return 0;
        }

        System.out.println("Email '" + email + "' found. Attempting to unsubscribe.");
        // 3. Call the unsubscribe script.
        int scriptResult = runUnsubscribeScript(email);

        // 4. Check script result and return final status.
        if (scriptResult == 0) {
            return 1; // Script succeeded
        } else {
            return 0; // Script failed
        }
    }

    public static void main(String[] args) {
        String[] testEmails = {
            "test@example.com",       // Exists, script will succeed
            "fail@example.com",       // Exists, script will fail
            "notfound@example.com",   // Does not exist
            "invalid-email",          // Invalid format
            "subscriber@newsletter.org"// Exists, script will succeed
        };

        int[] expectedResults = {1, 0, 0, 0, 1};

        for (int i = 0; i < testEmails.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + ": " + testEmails[i] + " ---");
            int result = unsubscribeEmail(testEmails[i]);
            System.out.println("Result: " + result + " (Expected: " + expectedResults[i] + ")");
            System.out.println(result == expectedResults[i] ? "Status: PASSED" : "Status: FAILED");
            System.out.println();
        }
    }
}