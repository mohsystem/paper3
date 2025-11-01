public class Lse109 {

    // A whitelist pattern for valid usernames.
    // Allows alphanumeric characters, underscore, dot, and hyphen.
    // This helps prevent command injection and other injection attacks.
    private static final java.util.regex.Pattern USERNAME_PATTERN = java.util.regex.Pattern.compile("^[a-zA-Z0-9_.-]+$");
    private static final String PASSWD_FILE = "/etc/passwd";

    /**
     * Finds and returns the line for a given username from /etc/passwd.
     * This implementation reads the file directly to avoid command injection vulnerabilities
     * that would arise from executing external shell commands with user input.
     *
     * @param username The username to search for.
     * @return The line from /etc/passwd corresponding to the user, or an empty string if not found or invalid.
     */
    public static String getUserInfo(String username) {
        if (username == null || !USERNAME_PATTERN.matcher(username).matches()) {
            System.err.println("Error: Invalid username format provided.");
            return "";
        }

        try (java.io.BufferedReader reader = new java.io.BufferedReader(new java.io.FileReader(PASSWD_FILE))) {
            String line;
            while ((line = reader.readLine()) != null) {
                // /etc/passwd format is username:password:UID:GID:...
                // We split only on the first colon to get the username part.
                String[] parts = line.split(":", 2);
                if (parts.length > 0 && parts[0].equals(username)) {
                    return line;
                }
            }
        } catch (java.io.IOException e) {
            System.err.println("Error reading " + PASSWD_FILE + ": " + e.getMessage());
        } catch (SecurityException e) {
            System.err.println("Error: Insufficient permissions to read " + PASSWD_FILE + ". " + e.getMessage());
        }

        return ""; // Return empty string if user is not found
    }

    public static void main(String[] args) {
        if (args.length != 1) {
            System.out.println("Usage: java Lse109 <username>");
            System.out.println("\n--- Running Internal Test Cases ---");
            runTests();
            return;
        }

        String username = args[0];
        String userInfo = getUserInfo(username);
        if (!userInfo.isEmpty()) {
            System.out.println(userInfo);
        } else {
            System.out.println("User '" + username + "' not found or input was invalid.");
        }
    }
    
    private static void runTests() {
        String[] testUsernames = {
            "root",                 // Test case 1: A common existing user
            "daemon",               // Test case 2: Another common user
            "nonexistentuser12345", // Test case 3: A non-existent user
            "root;whoami",          // Test case 4: A malicious input pattern
            ""                      // Test case 5: An empty input
        };

        for (String testUser : testUsernames) {
            System.out.println("Searching for user: \"" + testUser + "\"");
            String result = getUserInfo(testUser);
            if (!result.isEmpty()) {
                System.out.println("Found: " + result);
            } else {
                System.out.println("Result: Not found or invalid input.");
            }
            System.out.println("---------------------------------");
        }
    }
}