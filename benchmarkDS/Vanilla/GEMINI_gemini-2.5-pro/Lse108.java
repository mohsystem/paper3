import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

public class Lse108 {

    /**
     * Executes the command 'getent passwd <username>' and captures its output.
     * This implementation simulates the C-style system() call by using a shell
     * to handle output redirection to a temporary file, which is then read.
     *
     * @param username The username to look up.
     * @return The output from the command, or an empty string if an error occurs or the command has no output.
     */
    public static String getUserInfo(String username) {
        // Sanitize username to prevent command injection. A simple approach:
        // allow only alphanumeric characters, dashes, and underscores.
        if (username == null || !username.matches("^[a-zA-Z0-9_-]*$")) {
            return ""; // Return empty for invalid or potentially malicious usernames
        }

        String tempFileName = "java_getent_output.tmp";
        File tempFile = new File(tempFileName);
        String command = "getent passwd " + username;

        try {
            ProcessBuilder pb = new ProcessBuilder("sh", "-c", command);
            pb.redirectOutput(tempFile);
            Process process = pb.start();
            process.waitFor();

            if (tempFile.exists()) {
                return new String(Files.readAllBytes(Paths.get(tempFileName)));
            }
        } catch (IOException | InterruptedException e) {
            // e.printStackTrace(); // For debugging
            return "";
        } finally {
            if (tempFile.exists()) {
                tempFile.delete();
            }
        }
        return "";
    }

    public static void main(String[] args) {
        List<String> testUsernames = new ArrayList<>();
        testUsernames.add("root");
        testUsernames.add("daemon");
        testUsernames.add("nonexistentuser12345");
        
        String currentUser = System.getenv("USER");
        if (currentUser != null && !currentUser.isEmpty()) {
            testUsernames.add(currentUser);
        } else {
            testUsernames.add("nobody"); // Fallback if USER env var is not set
        }
        
        testUsernames.add("");

        System.out.println("--- Java Test Cases ---");
        for (String user : testUsernames) {
            System.out.println("Testing for user: \"" + user + "\"");
            String output = getUserInfo(user);
            if (output.isEmpty()) {
                System.out.println("Output: <No output or user not found>");
            } else {
                System.out.print("Output: " + output);
            }
            System.out.println("-------------------------");
        }
    }
}