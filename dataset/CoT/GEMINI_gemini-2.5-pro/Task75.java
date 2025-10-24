import java.util.Optional;

public class Task75 {

    /**
     * Terminates a process with the given PID using the ProcessHandle API (Java 9+).
     *
     * @param pid The Process ID of the process to terminate.
     */
    public static void terminateProcess(long pid) {
        Optional<ProcessHandle> processHandleOptional = ProcessHandle.of(pid);
        
        // Use ifPresentOrElse for cleaner handling of found/not-found cases
        processHandleOptional.ifPresentOrElse(
            processHandle -> {
                System.out.println("Found process with PID: " + pid + ". Attempting to terminate...");
                // destroyForcibly() is equivalent to SIGKILL
                boolean terminated = processHandle.destroyForcibly();
                if (terminated) {
                    System.out.println("Successfully sent termination signal to PID: " + pid);
                } else {
                    // This can happen if the process terminated between the check and the call,
                    // or due to insufficient permissions.
                    System.out.println("Failed to terminate PID: " + pid + ". The process may have already exited or you may lack permissions.");
                }
            },
            () -> {
                // This runs if the Optional is empty
                System.out.println("Error: Could not find any process with PID: " + pid);
            }
        );
    }

    public static void main(String[] args) {
        /*
         --- Test Cases ---
         To run the test cases, compile this file (javac Task75.java) and run from the command line.
         You will need to find a valid PID on your system to test case 1.
         - On Linux/macOS, you can run `sleep 100 &` to get a background process and its PID.
         - On Windows, you can open Notepad and find its PID in Task Manager (Details tab).

         1. Kill a valid process (e.g., a process with PID 1234):
            java Task75 1234

         2. Attempt to kill a non-existent process:
            java Task75 999999

         3. Attempt to kill a privileged/system process (e.g., PID 1 on Linux):
            java Task75 1
            (This should fail due to lack of permissions)

         4. Provide non-numeric input to see input validation:
            java Task75 abc

         5. Provide an invalid PID number:
            java Task75 -10
        */

        if (args.length != 1) {
            System.out.println("Usage: java Task75 <PID>");
            return;
        }

        try {
            long pid = Long.parseLong(args[0]);
            if (pid <= 0) {
                 System.out.println("Error: PID must be a positive integer.");
                 return;
            }
            terminateProcess(pid);
        } catch (NumberFormatException e) {
            System.out.println("Error: Invalid PID '" + args[0] + "'. Please provide a numeric value.");
        }
    }
}