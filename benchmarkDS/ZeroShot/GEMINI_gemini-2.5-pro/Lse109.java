import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.regex.Pattern;

public class Lse109 {

    // A regex for valid usernames. Typically, POSIX usernames don't contain special characters
    // beyond a-z, A-Z, 0-9, underscore, and hyphen. They also have length restrictions.
    private static final Pattern VALID_USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_-]{1,32}$");
    private static final String PASSWD_FILE = "/etc/passwd";

    /**
     * Securely retrieves a user's information from /etc/passwd by parsing the file.
     * This avoids command injection vulnerabilities.
     *
     * @param username The username to search for.
     * @return A string containing the user's line from /etc/passwd, or a message if not found or invalid.
     */
    public static String getUserInfo(String username) {
        if (username == null || !VALID_USERNAME_PATTERN.matcher(username).matches()) {
            return "Error: Invalid username format provided.";
        }
        
        if (!Files.exists(Paths.get(PASSWD_FILE))) {
            return "Error: " + PASSWD_FILE + " not found. This program is intended for Unix-like systems.";
        }

        try (BufferedReader reader = new BufferedReader(new FileReader(PASSWD_FILE))) {
            String line;
            while ((line = reader.readLine()) != null) {
                // The format of /etc/passwd is `username:password:UID:GID:GECOS:home:shell`
                String[] parts = line.split(":", 2);
                if (parts.length > 0 && parts[0].equals(username)) {
                    return line;
                }
            }
        } catch (IOException e) {
            return "Error: Failed to read " + PASSWD_FILE + ". " + e.getMessage();
        }

        return "User '" + username + "' not found.";
    }

    public static void main(String[] args) {
        // Handle command line argument if provided
        if (args.length > 0) {
            System.out.println("Searching for user from command line argument: " + args[0]);
            System.out.println(getUserInfo(args[0]));
            System.out.println("\n--- Running built-in test cases ---\n");
        }

        String[] testUsers = {
            "root",                 // A user that should exist on most systems
            "nobody",               // Another common user
            "nonexistentuser123",   // A user that should not exist
            "daemon",               // A common system user
            "user;rm-rf/"           // An invalid/malicious username attempt
        };

        for (String user : testUsers) {
            System.out.println("Searching for user: " + user);
            String result = getUserInfo(user);
            System.out.println("Result: " + result);
            System.out.println("--------------------");
        }
    }
}