
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;

public class Task66 {
    private static final Set<String> ALLOWED_COMMANDS = new HashSet<>(Arrays.asList(
        "echo", "date", "pwd", "whoami", "ls"
    ));
    
    private static final Pattern SAFE_ARGUMENT_PATTERN = Pattern.compile("^[a-zA-Z0-9._/\\\\-]+$");
    private static final int MAX_COMMAND_LENGTH = 256;
    private static final int MAX_ARGUMENTS = 10;
    
    public static String executeCommand(String userInput) {
        if (userInput == null || userInput.trim().isEmpty()) {
            return "Error: Empty command";
        }
        
        if (userInput.length() > MAX_COMMAND_LENGTH) {
            return "Error: Command too long";
        }
        
        String[] parts = userInput.trim().split("\\\\s+");
        
        if (parts.length == 0) {
            return "Error: Invalid command";
        }
        
        if (parts.length > MAX_ARGUMENTS + 1) {
            return "Error: Too many arguments";
        }
        
        String command = parts[0];
        
        if (!ALLOWED_COMMANDS.contains(command)) {
            return "Error: Command not allowed. Allowed commands: " + ALLOWED_COMMANDS;
        }
        
        for (int i = 1; i < parts.length; i++) {
            if (!SAFE_ARGUMENT_PATTERN.matcher(parts[i]).matches()) {
                return "Error: Invalid characters in argument";
            }
        }
        
        try {
            ProcessBuilder pb = new ProcessBuilder(parts);
            pb.redirectErrorStream(true);
            Process process = pb.start();
            
            StringBuilder output = new StringBuilder();
            try (BufferedReader reader = new BufferedReader(
                    new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append("\\n");
                }
            }
            
            int exitCode = process.waitFor();
            
            if (exitCode != 0) {
                return "Error: Command exited with code " + exitCode + "\\n" + output.toString();
            }
            
            return output.toString();
            
        } catch (Exception e) {
            return "Error: Failed to execute command: " + e.getClass().getSimpleName();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test 1 - echo hello:");
        System.out.println(executeCommand("echo hello"));
        
        System.out.println("\\nTest 2 - pwd:");
        System.out.println(executeCommand("pwd"));
        
        System.out.println("\\nTest 3 - Invalid command (rm):");
        System.out.println(executeCommand("rm file.txt"));
        
        System.out.println("\\nTest 4 - Invalid characters:");
        System.out.println(executeCommand("echo $(whoami)"));
        
        System.out.println("\\nTest 5 - Empty command:");
        System.out.println(executeCommand(""));
    }
}
