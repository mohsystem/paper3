import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

public class Task75 {

    /**
     * Terminates a process by its Process ID (PID).
     * This method is OS-dependent and works on Windows, Linux, and macOS.
     *
     * @param pid The PID of the process to terminate.
     * @return true if the termination command was executed successfully, false otherwise.
     */
    public static boolean terminateProcess(long pid) {
        if (pid <= 0) {
            return false;
        }
        String os = System.getProperty("os.name").toLowerCase();
        String command;

        if (os.contains("win")) {
            command = "taskkill /F /PID " + pid;
        } else if (os.contains("nix") || os.contains("nux") || os.contains("mac")) {
            command = "kill -9 " + pid;
        } else {
            System.err.println("Unsupported operating system.");
            return false;
        }

        try {
            Process process = Runtime.getRuntime().exec(command);
            // We wait for the command to complete. A 0 exit code typically means success.
            // If the process to be killed does not exist, the command will fail with a non-zero exit code.
            int exitCode = process.waitFor();
            return exitCode == 0;
        } catch (IOException | InterruptedException e) {
            // This catches errors in executing the command itself, not whether the kill was successful.
            return false;
        }
    }
    
    /**
     * Starts a dummy background process that runs for a while.
     * Required for automated testing.
     * @return The Process object of the new process, or null on failure.
     */
    private static Process startDummyProcess() {
        try {
            String os = System.getProperty("os.name").toLowerCase();
            ProcessBuilder pb;
            if (os.contains("win")) {
                // The 'timeout' command waits for the specified duration.
                pb = new ProcessBuilder("cmd", "/c", "timeout", "60");
            } else {
                // The 'sleep' command waits for the specified duration.
                pb = new ProcessBuilder("sleep", "60");
            }
            return pb.start();
        } catch (IOException e) {
            System.err.println("Failed to start dummy process: " + e.getMessage());
            return null;
        }
    }


    public static void main(String[] args) {
        if (args.length > 0) {
            try {
                long pid = Long.parseLong(args[0]);
                System.out.println("Attempting to terminate process with PID: " + pid);
                if (terminateProcess(pid)) {
                    System.out.println("Termination command for process " + pid + " sent successfully.");
                } else {
                    System.out.println("Failed to terminate process " + pid + ". It might not exist or you may lack permissions.");
                }
            } catch (NumberFormatException e) {
                System.err.println("Invalid PID provided. Please enter a numeric PID.");
            }
        } else {
            System.out.println("No PID provided. Running built-in test cases...");

            // Test Case 1: Terminate a valid, running process
            System.out.println("\n--- Test Case 1: Terminate a valid process ---");
            Process p1 = startDummyProcess();
            if (p1 != null && p1.isAlive()) {
                long pid1 = p1.pid();
                System.out.println("Started dummy process with PID: " + pid1);
                boolean result1 = terminateProcess(pid1);
                System.out.println("Termination result: " + (result1 ? "SUCCESS" : "FAILURE"));
                try { Thread.sleep(100); } catch (InterruptedException e) {} // Give OS time to update status
                System.out.println("Is process still alive? " + p1.isAlive());

                // Test Case 5: Attempt to terminate the same, now-terminated process
                System.out.println("\n--- Test Case 5: Re-terminate the same process ---");
                System.out.println("Attempting to terminate PID " + pid1 + " again.");
                boolean result5 = terminateProcess(pid1);
                System.out.println("Second termination result: " + (result5 ? "SUCCESS" : "FAILURE (Expected)"));

            } else {
                System.out.println("Skipping Test 1 & 5 as dummy process could not be started or is not alive.");
            }
            
            // Test Case 2: Terminate another valid process
            System.out.println("\n--- Test Case 2: Terminate another valid process ---");
            Process p2 = startDummyProcess();
            if (p2 != null && p2.isAlive()) {
                long pid2 = p2.pid();
                System.out.println("Started second dummy process with PID: " + pid2);
                boolean result2 = terminateProcess(pid2);
                System.out.println("Termination result: " + (result2 ? "SUCCESS" : "FAILURE"));
            } else {
                 System.out.println("Skipping Test 2 as dummy process could not be started.");
            }

            // Test Case 3: Attempt to terminate a non-existent process
            System.out.println("\n--- Test Case 3: Terminate a non-existent process ---");
            long nonExistentPid = 99999;
            System.out.println("Attempting to terminate non-existent PID: " + nonExistentPid);
            boolean result3 = terminateProcess(nonExistentPid);
            System.out.println("Termination result: " + (result3 ? "SUCCESS" : "FAILURE (Expected)"));
            
            // Test Case 4: Attempt to terminate an invalid PID (<= 0)
            System.out.println("\n--- Test Case 4: Terminate an invalid PID (e.g., 0) ---");
            long invalidPid = 0;
            System.out.println("Attempting to terminate invalid PID: " + invalidPid);
            boolean result4 = terminateProcess(invalidPid);
            System.out.println("Termination result: " + (result4 ? "SUCCESS" : "FAILURE (Expected)"));
        }
    }
}