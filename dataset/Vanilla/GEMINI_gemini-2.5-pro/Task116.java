public class Task116 {

    /*
    Dropping user privileges (e.g., via setuid/setgid) is a low-level
    operating system operation that is not supported by the standard Java SE API.
    Java is designed to be platform-independent, and such operations are
    highly specific to POSIX-compliant systems (like Linux and macOS).

    To achieve this in Java, one would typically need to use the Java Native
    Interface (JNI) to call native C code that can perform the setuid/setgid
    system calls. This approach breaks the "single source code file" requirement.

    Below is a conceptual simulation. It does not actually change the OS user ID
    of the running process. It only checks the username property. To see the
    difference in output, run it normally and then run with `sudo`.
    */

    /**
     * Simulates performing a privileged action and then dropping privileges.
     * @param targetUsername The user whose password we are pretending to change.
     * @param newPassword The new password (unused in this simulation).
     */
    public static void simulatePrivilegeDrop(String targetUsername, String newPassword) {
        System.out.println("--- Starting simulated process for user '" + targetUsername + "' ---");

        String currentOsUser = System.getProperty("user.name");
        System.out.println("Initial OS user name: " + currentOsUser);

        boolean isPrivileged = "root".equals(currentOsUser);

        // Step 1: Simulate privileged operation
        System.out.println("\nStep 1: Performing action as user '" + currentOsUser + "'.");
        if (isPrivileged) {
            System.out.println("PRIVILEGED: Successfully changed password for user '" + targetUsername + "'.");
        } else {
            System.out.println("NON-PRIVILEGED: Cannot change password for '" + targetUsername + "'. Action would fail.");
        }

        // Step 2: Simulate dropping privileges
        System.out.println("\nStep 2: Simulating privilege drop.");
        if (isPrivileged) {
            // In a real scenario, this is where a native call to setuid() would happen.
            String targetUser = System.getenv("SUDO_USER");
            if (targetUser == null || targetUser.isEmpty()) {
                targetUser = "nobody"; // Fallback non-privileged user
            }
            System.out.println("Simulating drop to user '" + targetUser + "'.");
            // We change our internal flag to represent the dropped privileges state.
            isPrivileged = false; 
            System.out.println("SUCCESS: Privileges conceptually dropped.");
        } else {
            System.out.println("Not running as root, no privileges to drop.");
        }

        // Step 3: Attempt privileged operation again
        System.out.println("\nStep 3: Attempting action with simulated dropped privileges.");
        if (isPrivileged) {
            // This block should not be reached if the drop was successful.
            System.out.println("ERROR: Privileges were not dropped correctly.");
        } else {
            System.out.println("FAILED: Insufficient privileges to change password for '" + targetUsername + "'.");
        }

        System.out.println("--- Process finished ---\n");
    }

    public static void main(String[] args) {
        System.out.println("Running Privilege Drop *Simulation* in Java");
        System.out.println("NOTE: This Java code only simulates the process and does not change the OS user ID.");
        
        String[] usersToUpdate = {"alice", "bob", "charlie", "david", "eve"};

        for (int i = 0; i < usersToUpdate.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            simulatePrivilegeDrop(usersToUpdate[i], "new_secure_password_123");
        }
    }
}