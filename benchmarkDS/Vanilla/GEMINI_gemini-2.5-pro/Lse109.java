import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

public class Lse109 {

    /**
     * Executes 'cat /etc/passwd | grep <username>' and returns the output.
     * @param username The username to search for.
     * @return A string containing the user's entry from /etc/passwd, or a status message.
     */
    public static String getUserInfo(String username) {
        StringBuilder output = new StringBuilder();
        // Use a more specific grep to avoid partial matches (e.g., 'user' matching 'otheruser')
        // by anchoring the search to the beginning of the line and ending with a colon.
        String command = "cat /etc/passwd | grep '^" + username + ":'";
        Process process;

        try {
            // Use "sh -c" to correctly interpret the command pipeline.
            process = Runtime.getRuntime().exec(new String[]{"sh", "-c", command});

            // Read the standard output of the command.
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append(System.lineSeparator());
                }
            }

            process.waitFor();

            if (output.length() == 0) {
                return "User '" + username + "' not found.\n";
            }

        } catch (IOException | InterruptedException e) {
            return "Failed to execute command: " + e.getMessage() + "\n";
        }
        return output.toString();
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            // If a username is provided via command line argument, use it.
            System.out.print(getUserInfo(args[0]));
        } else {
            // Otherwise, run a set of predefined test cases.
            System.out.println("No command line argument provided. Running built-in test cases:");
            System.out.println("===============================================================");
            String[] testCases = {"root", "daemon", "bin", "sys", "nonexistentuser123"};
            for (String user : testCases) {
                System.out.println("\n--- Testing for user: '" + user + "' ---");
                System.out.print(getUserInfo(user));
            }
        }
    }
}