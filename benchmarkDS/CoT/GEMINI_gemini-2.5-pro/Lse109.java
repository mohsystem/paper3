import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

public class Lse109 {

    /**
     * Finds and returns a user's information from /etc/passwd.
     * This method safely reads the file line by line to prevent command injection
     * vulnerabilities that would arise from executing shell commands with user input.
     *
     * @param username The username to search for.
     * @return The full line from /etc/passwd for the user, or a "not found" message.
     */
    public static String getUserInfo(String username) {
        // Basic validation to prevent nulls or empty strings
        if (username == null || username.trim().isEmpty() || username.contains("/") || username.contains("..")) {
            return "Invalid username provided.";
        }

        String passwdFile = "/etc/passwd";
        String searchPrefix = username + ":";
        
        // Using try-with-resources to ensure the reader is closed automatically
        try (BufferedReader reader = new BufferedReader(new FileReader(passwdFile))) {
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.startsWith(searchPrefix)) {
                    return line;
                }
            }
        } catch (IOException e) {
            return "Error reading password file: " + e.getMessage();
        }

        return "User '" + username + "' not found.";
    }

    public static void main(String[] args) {
        // The original prompt asked for command-line argument processing.
        // If arguments are provided, we use them. Otherwise, we run test cases.
        if (args.length > 0) {
            String username = args[0];
            System.out.println("Searching for user from command line: " + username);
            System.out.println(getUserInfo(username));
        } else {
            // Running 5 test cases as requested.
            System.out.println("--- Running Test Cases ---");
            String[] testUsers = {"root", "daemon", "nobody", "nonexistentuser123", "adm"};
            
            for (String user : testUsers) {
                System.out.println("\nSearching for user: " + user);
                String result = getUserInfo(user);
                System.out.println("Result: " + result);
            }
            System.out.println("\n--- Test Cases Finished ---");
            System.out.println("You can also run this program with a command-line argument, e.g., java Lse109 root");
        }
    }
}