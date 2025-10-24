
import java.io.IOException;

public class Task75 {
    /**
     * Terminates a process by PID with security validations
     * @param pid Process ID to terminate
     * @return true if successful, false otherwise
     */
    public static boolean terminateProcess(String pid) {
        // Input validation - prevent injection attacks
        if (pid == null || pid.trim().isEmpty()) {
            System.err.println("Error: PID cannot be null or empty");
            return false;
        }
        
        // Validate PID is numeric only (prevent command injection)
        if (!pid.matches("^[0-9]+$")) {
            System.err.println("Error: PID must contain only digits");
            return false;
        }
        
        // Convert to long to validate range
        try {
            long pidValue = Long.parseLong(pid);
            if (pidValue <= 0) {
                System.err.println("Error: PID must be positive");
                return false;
            }
            
            // Additional check: prevent terminating system critical PIDs (1-1000)
            if (pidValue <= 1000) {
                System.err.println("Warning: Cannot terminate system process (PID <= 1000)");
                return false;
            }
        } catch (NumberFormatException e) {
            System.err.println("Error: Invalid PID format");
            return false;
        }
        
        try {
            String os = System.getProperty("os.name").toLowerCase();
            ProcessBuilder pb;
            
            if (os.contains("win")) {
                // Windows: Use taskkill with /F flag
                pb = new ProcessBuilder("taskkill", "/F", "/PID", pid);
            } else {
                // Unix/Linux/Mac: Use kill command
                pb = new ProcessBuilder("kill", "-9", pid);
            }
            
            // Security: Redirect error stream to prevent information leakage
            pb.redirectErrorStream(true);
            
            Process process = pb.start();
            int exitCode = process.waitFor();
            
            if (exitCode == 0) {
                System.out.println("Process " + pid + " terminated successfully");
                return true;
            } else {
                System.err.println("Failed to terminate process " + pid);
                return false;
            }
            
        } catch (IOException e) {
            System.err.println("Error: Unable to execute termination command");
            return false;
        } catch (InterruptedException e) {
            System.err.println("Error: Process termination interrupted");
            Thread.currentThread().interrupt();
            return false;
        } catch (SecurityException e) {
            System.err.println("Error: Insufficient permissions");
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test cases with validation
        System.out.println("=== Process Termination Test Cases ===\\n");
        
        // Test Case 1: Valid PID format
        System.out.println("Test 1: Valid PID (12345)");
        terminateProcess("12345");
        System.out.println();
        
        // Test Case 2: Invalid PID (negative)
        System.out.println("Test 2: Negative PID (-100)");
        terminateProcess("-100");
        System.out.println();
        
        // Test Case 3: Invalid PID (non-numeric)
        System.out.println("Test 3: Non-numeric PID (abc123)");
        terminateProcess("abc123");
        System.out.println();
        
        // Test Case 4: System process protection
        System.out.println("Test 4: System process (PID 1)");
        terminateProcess("1");
        System.out.println();
        
        // Test Case 5: Empty/null input
        System.out.println("Test 5: Empty PID");
        terminateProcess("");
        System.out.println();
        
        // Command line argument handling
        if (args.length > 0) {
            System.out.println("=== Command Line Execution ===");
            for (String pid : args) {
                System.out.println("Attempting to terminate PID: " + pid);
                terminateProcess(pid);
            }
        }
    }
}
