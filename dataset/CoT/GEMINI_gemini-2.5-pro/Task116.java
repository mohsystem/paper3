import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task116 {

    /**
     * NOTE: The Java Virtual Machine (JVM) abstracts away the underlying OS.
     * Standard Java APIs do not provide a way to change the user identity (UID/GID)
     * of the running process. This is a security feature to maintain platform independence
     * and the integrity of the JVM's security model.
     *
     * Therefore, this implementation SIMULATES the desired behavior by executing
     * system commands using a helper tool like `sudo`. This is fundamentally
     * different from the C/C++/Python examples which change the UID of the
     * current process itself.
     */

    /**
     * Simulates a privileged operation, like changing a user's password.
     * In a real-world scenario, this would involve a secure, non-shell-based method.
     * Here, we just print a message to simulate the action.
     * @param username The username for the password change.
     * @param password The new password (in a real app, use char[]).
     * @return true if the simulation was successful, false otherwise.
     */
    public static boolean changePassword(String username, String password) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Error: Username and password cannot be empty.");
            return false;
        }

        System.out.println("[PRIVILEGED] Simulating password change for user: " + username);
        // In a real application, you would use a secure method to call a system utility.
        // For demonstration, we just show success.
        System.out.println("[PRIVILEGED] Operation completed successfully.");
        return true;
    }

    /**
     * Simulates dropping privileges by executing a new command as a non-privileged user.
     * The original Java process *does not* change its own user ID.
     * It simply launches a new, separate process with a different user ID.
     *
     * @param nonPrivilegedUser The user to switch to for new commands (e.g., "nobody").
     */
    public static void dropPrivileges(String nonPrivilegedUser) {
        System.out.println("\nAttempting to drop privileges by running subsequent commands as '" + nonPrivilegedUser + "'...");
        System.out.println("NOTE: The main Java process remains running as the original user.");

        // We demonstrate the dropped privilege by running 'whoami' as the new user.
        List<String> command = new ArrayList<>(Arrays.asList("sudo", "-u", nonPrivilegedUser, "whoami"));
        
        try {
            System.out.println("Executing command: " + String.join(" ", command));
            ProcessBuilder pb = new ProcessBuilder(command);
            Process process = pb.start();

            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line;
            System.out.print("Output of 'whoami' as new user: ");
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
            }
            
            int exitCode = process.waitFor();
            if (exitCode == 0) {
                System.out.println("Successfully executed a command as '" + nonPrivilegedUser + "'. Privileges are conceptually 'dropped' for new tasks.");
            } else {
                 System.out.println("Failed to execute command as '" + nonPrivilegedUser + "'. Exit code: " + exitCode);
                 // Print error stream for diagnostics
                 BufferedReader errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream()));
                 System.out.println("Error details:");
                 while ((line = errorReader.readLine()) != null) {
                    System.out.println(line);
                 }
            }

        } catch (IOException | InterruptedException e) {
            System.err.println("An error occurred while trying to run command as non-privileged user: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        System.out.println("### Java Privilege Dropping Simulation ###");
        System.out.println("This program must be run with sudo (e.g., 'sudo java Task116') for the simulation to work.");

        // Get current user (of the Java process itself)
        String currentUser = System.getProperty("user.name");
        System.out.println("Java process is running as user: '" + currentUser + "'");

        if (!"root".equals(currentUser)) {
            System.out.println("Warning: This simulation is best run as 'root' to demonstrate privilege changes.");
        }

        // --- Test Cases ---

        // Test Case 1: Successful privileged operation
        System.out.println("\n--- Test Case 1: Perform privileged operation as root ---");
        changePassword("testuser1", "newPassword123");

        // Test Case 2: Drop privileges
        System.out.println("\n--- Test Case 2: Drop privileges to a non-root user ---");
        // On many Linux systems, 'nobody' is a standard unprivileged user.
        // On others, you might need to use a different user like 'daemon'.
        String nonPrivUser = "nobody"; 
        dropPrivileges(nonPrivUser);
        
        // Test Case 3: Attempt privileged operation after "dropping" privileges.
        // The Java process is still root, so this will appear to succeed.
        // This highlights the key difference and limitation in Java.
        System.out.println("\n--- Test Case 3: Attempt privileged operation again from main process ---");
        System.out.println("NOTE: This will still succeed because the main Java process never dropped its original privileges.");
        changePassword("testuser2", "anotherPassword456");

        // Test Case 4: Invalid user for password change
        System.out.println("\n--- Test Case 4: Call with empty username ---");
        changePassword("", "password");
        
        // Test Case 5: Non-existent user for privilege drop
        System.out.println("\n--- Test Case 5: Attempt to drop to a non-existent user ---");
        dropPrivileges("nonexistentuser12345");
    }
}