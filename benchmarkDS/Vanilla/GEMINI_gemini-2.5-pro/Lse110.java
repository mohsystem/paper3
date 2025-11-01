import java.io.IOException;

public class Lse110 {
    /**
     * Runs the 'id <username>' command and returns its exit status.
     * @param username The username to look up.
     * @return The exit status of the 'id' command (0 for success, non-zero for failure).
     */
    public static int runIdCommand(String username) {
        // Prevent command injection by ensuring username is a single argument
        if (username == null || username.isEmpty() || username.contains(" ")) {
            return -1; // Invalid username format
        }

        try {
            // Use ProcessBuilder for better security and control
            ProcessBuilder pb = new ProcessBuilder("id", username);
            
            // Optional: redirect output and error streams to avoid printing them
            pb.redirectOutput(ProcessBuilder.Redirect.DISCARD);
            pb.redirectError(ProcessBuilder.Redirect.DISCARD);

            Process process = pb.start();
            
            // Wait for the command to complete and return its exit code
            return process.waitFor();
        } catch (IOException | InterruptedException e) {
            // e.printStackTrace(); // For debugging
            return -1; // Indicate that the command could not be run
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            "root",                 // Test case 1: Should exist on most systems (success)
            "daemon",               // Test case 2: Should also exist on most systems (success)
            "nonexistentuser12345", // Test case 3: Should not exist (failure)
            "another_fake_user_xyz",// Test case 4: Should not exist (failure)
            ""                      // Test case 5: Invalid input (failure)
        };

        System.out.println("--- Running Java Test Cases ---");
        for (String user : testCases) {
            int exitStatus = runIdCommand(user);
            System.out.println("Command 'id " + (user.isEmpty() ? "<empty>" : user) + "' exited with status: " + exitStatus);
        }
    }
}