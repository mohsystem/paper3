
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class Task158 {
    private static final Set<String> ALLOWED_COMMANDS = new HashSet<>(
        Arrays.asList("echo", "date", "pwd", "whoami", "hostname")
    );
    
    public static String executeCommand(String command, String argument) {
        try {
            // Validate command against whitelist
            if (!ALLOWED_COMMANDS.contains(command)) {
                return "Error: Command not allowed. Allowed commands: " + ALLOWED_COMMANDS;
            }
            
            // Sanitize argument - remove dangerous characters
            String sanitizedArg = sanitizeInput(argument);
            
            // Use ProcessBuilder for secure command execution
            ProcessBuilder processBuilder = new ProcessBuilder();
            processBuilder.command(command, sanitizedArg);
            
            Process process = processBuilder.start();
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            
            StringBuilder output = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            int exitCode = process.waitFor();
            if (exitCode != 0) {
                return "Error: Command execution failed with exit code " + exitCode;
            }
            
            return output.toString().trim();
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    private static String sanitizeInput(String input) {
        if (input == null) {
            return "";
        }
        // Remove potentially dangerous characters
        return input.replaceAll("[;&|`$()<>\\\\\\\\'\\"]", "");
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure OS Command Execution ===\\n");
        
        // Test case 1: Valid echo command
        System.out.println("Test 1 - Echo command:");
        System.out.println(executeCommand("echo", "Hello World"));
        System.out.println();
        
        // Test case 2: Date command
        System.out.println("Test 2 - Date command:");
        System.out.println(executeCommand("date", ""));
        System.out.println();
        
        // Test case 3: Invalid command (blocked)
        System.out.println("Test 3 - Invalid command (should be blocked):");
        System.out.println(executeCommand("rm", "-rf /"));
        System.out.println();
        
        // Test case 4: Command injection attempt (sanitized)
        System.out.println("Test 4 - Command injection attempt:");
        System.out.println(executeCommand("echo", "test; rm -rf /"));
        System.out.println();
        
        // Test case 5: Hostname command
        System.out.println("Test 5 - Hostname command:");
        System.out.println(executeCommand("hostname", ""));
    }
}
