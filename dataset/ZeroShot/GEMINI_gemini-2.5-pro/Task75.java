import java.io.IOException;
import java.util.Optional;
import java.lang.ProcessHandle;
import java.lang.ProcessBuilder;
import java.util.concurrent.TimeUnit;

public class Task75 {

    /**
     * Terminates a process identified by the given PID using a forceful signal (SIGKILL).
     *
     * @param pid The Process ID of the process to terminate.
     * @return true if the termination signal was successfully sent, false otherwise.
     */
    public static boolean terminateProcess(long pid) {
        // Security: Validate PID. PIDs must be positive.
        // Rejecting 0 and -1 prevents unintentionally sending signals to process groups
        // or all user processes.
        if (pid <= 0) {
            System.err.println("Error: Invalid PID " + pid + ". PID must be a positive number.");
            return false;
        }

        // Security: Prevent the process from terminating itself.
        if (pid == ProcessHandle.current().pid()) {
            System.err.println("Error: Attempting to terminate the current process is not allowed.");
            return false;
        }

        System.out.println("Attempting to terminate PID: " + pid);
        Optional<ProcessHandle> processHandleOptional = ProcessHandle.of(pid);

        if (processHandleOptional.isPresent()) {
            ProcessHandle processHandle = processHandleOptional.get();
            System.out.println("Process found. Forcibly terminating...");
            // .destroy() sends a graceful termination signal (SIGTERM).
            // .destroyForcibly() sends a forceful termination signal (SIGKILL).
            boolean result = processHandle.destroyForcibly();
            if (result) {
                System.out.println("Termination signal sent successfully to PID: " + pid);
            } else {
                // This can occur due to lack of permissions or if the process died
                // between the isPresent() check and this call.
                System.err.println("Failed to send termination signal to PID: " + pid);
            }
            return result;
        } else {
            System.err.println("Error: Process with PID " + pid + " not found.");
            return false;
        }
    }

    /**
     * The main method handles command-line arguments or runs a test suite.
     *
     * @param args Command-line arguments. If provided, the first argument is treated as a PID to terminate.
     */
    public static void main(String[] args) {
        if (args.length > 0) {
            // If a command-line argument is provided, use it as the PID.
            try {
                long pidToKill = Long.parseLong(args[0]);
                terminateProcess(pidToKill);
            } catch (NumberFormatException e) {
                System.err.println("Error: Invalid input. PID must be a numeric value.");
                System.exit(1);
            }
        } else {
            // Otherwise, run the built-in test cases.
            System.out.println("No PID provided. Running test cases...");
            runTestCases();
        }
    }

    private static void runTestCases() {
        System.out.println("\n--- Running Test Suite ---");

        // Test Case 1: Terminate a valid child process.
        Process childProcess = null;
        try {
            System.out.println("\n--- Test Case 1: Valid PID ---");
            String command = System.getProperty("os.name").startsWith("Windows") ? "timeout" : "sleep";
            childProcess = new ProcessBuilder(command, "300").start();
            long childPid = childProcess.pid();
            System.out.println("Started child process with PID: " + childPid);
            TimeUnit.SECONDS.sleep(1); // Wait a moment for the process to initialize.
            terminateProcess(childPid);
            TimeUnit.SECONDS.sleep(1); // Wait for termination to complete.
            System.out.println("Child process is alive after termination attempt: " + childProcess.isAlive());
        } catch (IOException | InterruptedException e) {
            System.err.println("Failed to create or manage child process for testing: " + e.getMessage());
        } finally {
            if (childProcess != null && childProcess.isAlive()) {
                childProcess.destroyForcibly(); // Cleanup for safety.
            }
        }

        // Test Case 2: Attempt to terminate a non-existent PID.
        System.out.println("\n--- Test Case 2: Non-existent PID ---");
        terminateProcess(99999); // A PID that is very unlikely to exist.

        // Test Case 3: Attempt to terminate PID 0 (rejected by validation).
        System.out.println("\n--- Test Case 3: Invalid PID (0) ---");
        terminateProcess(0);

        // Test Case 4: Attempt to terminate PID -1 (rejected by validation).
        System.out.println("\n--- Test Case 4: Invalid PID (-1) ---");
        terminateProcess(-1);

        // Test Case 5: Attempt to terminate the current process (self-termination).
        System.out.println("\n--- Test Case 5: Self-termination ---");
        terminateProcess(ProcessHandle.current().pid());

        System.out.println("\n--- Test Suite Finished ---");
    }
}