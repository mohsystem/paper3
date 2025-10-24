import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.regex.Pattern;

public class Task116 {

    // A pattern to validate usernames to be safe (alphanumeric, dashes, underscores)
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_-]{1,32}$");

    /**
     * Gets the current user ID (UID) on a POSIX-like system.
     * This is a workaround as pure Java does not provide a direct way to get the UID.
     * @return The UID as a long, or -1 on error.
     */
    private static long getCurrentUid() {
        // This method is OS-dependent and works on Linux/macOS.
        try {
            Process process = Runtime.getRuntime().exec("/usr/bin/id -u");
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line = reader.readLine();
                process.waitFor();
                if (line != null) {
                    return Long.parseLong(line.trim());
                }
            }
        } catch (IOException | InterruptedException | NumberFormatException e) {
            System.err.println("Warning: Could not determine UID. Assuming non-root. " + e.getMessage());
            return -1; // Indicate error or non-root status
        }
        return -1;
    }

    /**
     * Simulates performing a privileged operation and then dropping privileges.
     * In real Java, dropping privileges for the current JVM process is not possible
     * without using JNI/JNA to call native OS functions (e.g., setuid on Linux).
     *
     * @param username The OS username for the password change.
     * @param newPassword The new password (in a real scenario, should be handled securely).
     * @param dropToUser The non-privileged user to drop to.
     * @return true if the privileged operation was simulated successfully, false otherwise.
     */
    public static boolean changePasswordAndSimulatePrivilegeDrop(String username, String newPassword, String dropToUser) {
        if (!USERNAME_PATTERN.matcher(username).matches() || !USERNAME_PATTERN.matcher(dropToUser).matches()) {
            System.err.println("Error: Invalid username format.");
            return false;
        }

        long currentUid = getCurrentUid();
        System.out.println("Current UID: " + (currentUid == -1 ? "Unknown" : currentUid));

        if (currentUid != 0) {
            System.out.println("Operation failed: Must be run as root (UID 0) to change passwords.");
            return false;
        }

        // --- PRIVILEGED SECTION ---
        System.out.println("[PRIVILEGED] Running as root. Performing privileged operations.");
        System.out.println("[PRIVILEGED] Simulating: Changing password for user '" + username + "'.");
        // In a real application, you would securely call a system command like 'chpasswd'.
        // Example: ProcessBuilder pb = new ProcessBuilder("chpasswd"); ...

        System.out.println("[PRIVILEGED] Privileged operations complete.");
        // --- END PRIVILEGED SECTION ---


        // --- PRIVILEGE DROP SIMULATION ---
        System.out.println("\n--- Dropping privileges to user '" + dropToUser + "' ---");
        System.out.println("Java Limitation: The JVM does not support dropping privileges natively.");
        System.out.println("In a real-world scenario, a library like JNA or JNI would be used here");
        System.out.println("to call the native 'setuid' and 'setgid' system calls.");
        System.out.println("Simulation complete. The application would now be running as '" + dropToUser + "'.\n");
        // --- END SIMULATION ---

        return true;
    }

    public static void main(String[] args) {
        System.out.println("--- Privilege Drop Demonstration (Java Simulation) ---");
        System.out.println("NOTE: This program must be run with 'sudo' to demonstrate the privileged path.\n");

        // Test Case 1: A standard valid operation
        System.out.println("--- Test Case 1: Valid operation ---");
        boolean success1 = changePasswordAndSimulatePrivilegeDrop("testuser1", "newPass123!", "nobody");
        System.out.println("Operation successful: " + success1);
        // After this, any further privileged calls would fail.
        System.out.println("Attempting another privileged operation after simulated drop:");
        long uid = getCurrentUid();
        if (uid == 0) {
             System.out.println("Still root, as this is a simulation. A real drop would prevent this.");
        } else {
             System.out.println("No longer root.");
        }
        System.out.println("-------------------------------------------\n");


        // Test Case 2: Invalid username
        System.out.println("--- Test Case 2: Invalid username '; rm -rf /' ---");
        boolean success2 = changePasswordAndSimulatePrivilegeDrop("; rm -rf /", "password", "nobody");
        System.out.println("Operation successful: " + success2);
        System.out.println("-------------------------------------------\n");

        // Test Case 3: Empty username
        System.out.println("--- Test Case 3: Empty username ---");
        boolean success3 = changePasswordAndSimulatePrivilegeDrop("", "password", "nobody");
        System.out.println("Operation successful: " + success3);
        System.out.println("-------------------------------------------\n");
        
        // Test Case 4: Another valid user
        System.out.println("--- Test Case 4: Another valid user 'appadmin' ---");
        boolean success4 = changePasswordAndSimulatePrivilegeDrop("appadmin", "anotherSecurePass", "www-data");
        System.out.println("Operation successful: " + success4);
        System.out.println("-------------------------------------------\n");

        // Test Case 5: Invalid drop-to user
        System.out.println("--- Test Case 5: Invalid drop-to user 'bad&user' ---");
        boolean success5 = changePasswordAndSimulatePrivilegeDrop("testuser5", "pass", "bad&user");
        System.out.println("Operation successful: " + success5);
        System.out.println("-------------------------------------------\n");
    }
}