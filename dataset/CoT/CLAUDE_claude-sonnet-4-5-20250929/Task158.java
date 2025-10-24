
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.Arrays;
import java.util.List;
import java.util.Scanner;
import java.util.regex.Pattern;

public class Task158 {
    private static final Pattern SAFE_FILENAME_PATTERN = Pattern.compile("^[a-zA-Z0-9._-]+$");
    private static final List<String> ALLOWED_COMMANDS = Arrays.asList("echo", "date", "whoami");
    
    public static String executeCommand(String command, String argument) {
        try {
            // Validate command is in whitelist
            if (!ALLOWED_COMMANDS.contains(command)) {
                return "Error: Command not allowed. Allowed commands: " + ALLOWED_COMMANDS;
            }
            
            // Validate argument (no special characters for shell injection)
            if (argument != null && !argument.isEmpty() && !SAFE_FILENAME_PATTERN.matcher(argument).matches()) {
                return "Error: Invalid argument. Only alphanumeric characters, dots, hyphens, and underscores allowed.";
            }
            
            // Use ProcessBuilder for safe command execution (no shell interpretation)
            ProcessBuilder processBuilder;
            if (argument != null && !argument.isEmpty()) {
                processBuilder = new ProcessBuilder(command, argument);
            } else {
                processBuilder = new ProcessBuilder(command);
            }
            
            processBuilder.redirectErrorStream(true);
            Process process = processBuilder.start();
            
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            StringBuilder output = new StringBuilder();
            String line;
            
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            int exitCode = process.waitFor();
            if (exitCode != 0) {
                return "Command executed with exit code: " + exitCode + "\\n" + output.toString();
            }
            
            return output.toString().trim();
            
        } catch (Exception e) {
            return "Error executing command: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure OS Command Execution Demo ===\\n");
        
        // Test Case 1: Valid echo command with safe argument
        System.out.println("Test 1 - Echo command:");
        System.out.println(executeCommand("echo", "Hello_World"));
        System.out.println();
        
        // Test Case 2: Date command (no argument)
        System.out.println("Test 2 - Date command:");
        System.out.println(executeCommand("date", ""));
        System.out.println();
        
        // Test Case 3: Invalid command (not in whitelist)
        System.out.println("Test 3 - Invalid command:");
        System.out.println(executeCommand("rm", "file.txt"));
        System.out.println();
        
        // Test Case 4: Command injection attempt (should be blocked)
        System.out.println("Test 4 - Injection attempt:");
        System.out.println(executeCommand("echo", "test; rm -rf /"));
        System.out.println();
        
        // Test Case 5: Whoami command
        System.out.println("Test 5 - Whoami command:");
        System.out.println(executeCommand("whoami", ""));
        System.out.println();
    }
}
