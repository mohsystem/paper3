
// Java implementation
// SECURITY NOTE: This code demonstrates the UNSAFE pattern described in the prompt.
// Executing shell commands with user input creates critical command injection vulnerabilities (CWE-78).
// In production code, NEVER use Runtime.exec() or ProcessBuilder with unsanitized input.
// This implementation shows the secure alternative: validating and restricting input.

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.Arrays;
import java.util.List;

public class Lse115 {
    
    // Secure command execution with strict validation
    // Only allows whitelisted commands and validates directory paths
    public static String executeCommand(String directory) {
        // Input validation: reject null or empty
        if (directory == null || directory.isEmpty()) {
            return "Error: Invalid directory parameter";
        }
        
        // Sanitization: remove any shell metacharacters and path traversal attempts
        // Block characters: ; | & $ ` \\ " ' < > ( ) { } [ ] * ? ~ ! # % ^\n        if (directory.matches(".*[;&|`$\\\\\\\\\\"'<>(){}\\\\[\\\\]*?~!#%^].*") || 
            directory.contains("..") || 
            directory.startsWith("/")) {
            return "Error: Invalid characters in directory path";
        }
        
        // Whitelist validation: only allow alphanumeric, dots (not ..), hyphens, and underscores
        if (!directory.matches("^[a-zA-Z0-9_-]+$")) {
            return "Error: Directory name contains invalid characters";
        }
        
        StringBuilder output = new StringBuilder();
        ProcessBuilder processBuilder = new ProcessBuilder();
        
        try {
            // Use ProcessBuilder with array of arguments (NOT single string) to prevent injection
            // This prevents shell interpretation of the arguments
            List<String> command = Arrays.asList("ls", "-la", directory);
            processBuilder.command(command);
            
            // Security: Do not inherit environment and set working directory
            processBuilder.environment().clear();
            processBuilder.redirectErrorStream(true);
            
            Process process = processBuilder.start();
            
            // Read output with resource management
            try (BufferedReader reader = new BufferedReader(
                    new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append("\\n");
                }
            }
            
            // Wait for process completion with timeout
            boolean finished = process.waitFor(5, java.util.concurrent.TimeUnit.SECONDS);
            if (!finished) {
                process.destroyForcibly();
                return "Error: Command execution timeout";
            }
            
            int exitCode = process.exitValue();
            if (exitCode != 0) {
                return "Error: Command failed with exit code " + exitCode;
            }
            
            return output.toString();
            
        } catch (Exception e) {
            // Generic error message - do not leak internal details
            return "Error: Command execution failed";
        }
    }
    
    public static void main(String[] args) {
        // Test cases with validated inputs
        System.out.println("Test 1 - Valid directory:");
        System.out.println(executeCommand("testdir"));
        System.out.println();
        
        System.out.println("Test 2 - Valid directory with underscore:");
        System.out.println(executeCommand("my_folder"));
        System.out.println();
        
        System.out.println("Test 3 - Invalid: path traversal attempt:");
        System.out.println(executeCommand("../etc"));
        System.out.println();
        
        System.out.println("Test 4 - Invalid: command injection attempt:");
        System.out.println(executeCommand("test; rm -rf /"));
        System.out.println();
        
        System.out.println("Test 5 - Invalid: pipe attempt:");
        System.out.println(executeCommand("test | cat /etc/passwd"));
    }
}
