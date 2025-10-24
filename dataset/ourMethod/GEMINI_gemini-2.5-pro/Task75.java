import java.util.Optional;

public class Task75 {

    /**
     * Terminates a process by its Process ID (PID).
     *
     * @param pid The ID of the process to terminate.
     * @return true if a termination signal was successfully sent, false otherwise.
     */
    public static boolean terminateProcessById(long pid) {
        if (pid <= 0) {
            System.err.println("Error: PID must be a positive number.");
            return false;
        }

        try {
            Optional<ProcessHandle> processHandleOptional = ProcessHandle.of(pid);
            if (processHandleOptional.isPresent()) {
                ProcessHandle processHandle = processHandleOptional.get();
                System.out.println("Attempting to terminate process with PID: " + pid +
                        ", Command: " + processHandle.info().command().orElse("N/A"));
                
                // destroy() requests graceful termination.
                // destroyForcibly() would be equivalent to SIGKILL.
                return processHandle.destroy();
            } else {
                System.err.println("Error: Process with PID " + pid + " not found.");
                return false;
            }
        } catch (SecurityException e) {
            System.err.println("Error: Permission denied to terminate process with PID " + pid + ". " + e.getMessage());
            return false;
        } catch (Exception e) {
            System.err.println("An unexpected error occurred while terminating PID " + pid + ": " + e.getMessage());
            return false;
        }
    }

    /**
     * Helper function for running test cases.
     * @param pid The PID to test termination with.
     */
    private static void testTerminate(long pid) {
        System.out.println("\n--- Testing with PID: " + pid + " ---");
        if (terminateProcessById(pid)) {
            System.out.println("Result: Termination signal sent successfully.");
        } else {
            System.out.println("Result: Failed to send termination signal.");
        }
    }

    public static void main(String[] args) {
        if (args.length != 1) {
            System.out.println("Usage: java Task75 <PID>");
            System.out.println("\n--- Running Built-in Test Cases ---");
            // Note: These test cases use placeholder PIDs.
            // To test effectively, start a process (e.g., 'sleep 300' on Linux/macOS or 'timeout 300' on Windows)
            // and replace a placeholder with its actual PID.

            // Test 1: PID 1 (init/systemd/launchd), will likely fail due to permissions.
            testTerminate(1);

            // Test 2: A PID that is highly unlikely to exist.
            testTerminate(999999);

            // Test 3: An invalid PID (zero).
            testTerminate(0);

            // Test 4: An invalid PID (negative).
            testTerminate(-10);

            // Test 5: A placeholder for a real PID you can test with.
            testTerminate(12345);
            System.out.println("\n--- Test Cases Finished ---");
            return;
        }

        try {
            long pid = Long.parseLong(args[0]);
            if (terminateProcessById(pid)) {
                System.out.println("Termination signal was sent to process with PID: " + pid);
                // Note: The process may not terminate immediately.
            } else {
                System.out.println("Failed to terminate process with PID: " + pid);
            }
        } catch (NumberFormatException e) {
            System.err.println("Error: Invalid PID provided. It must be a valid number.");
        }
    }
}