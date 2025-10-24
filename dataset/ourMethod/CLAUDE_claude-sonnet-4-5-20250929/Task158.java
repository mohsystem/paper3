
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.regex.Pattern;

public class Task158 {
    private static final Pattern SAFE_INPUT_PATTERN = Pattern.compile("^[a-zA-Z0-9_.-]+$");
    private static final int MAX_INPUT_LENGTH = 100;
    private static final List<String> ALLOWED_COMMANDS = Arrays.asList("echo", "date", "whoami");
    
    public static String executeCommand(String command, String userInput) {
        if (command == null || userInput == null) {
            return "Error: Null input not allowed";
        }
        
        if (command.trim().isEmpty() || userInput.trim().isEmpty()) {
            return "Error: Empty input not allowed";
        }
        
        if (!ALLOWED_COMMANDS.contains(command)) {
            return "Error: Command not in whitelist";
        }
        
        if (userInput.length() > MAX_INPUT_LENGTH) {
            return "Error: Input exceeds maximum length";
        }
        
        if (!SAFE_INPUT_PATTERN.matcher(userInput).matches()) {
            return "Error: Input contains invalid characters";
        }
        
        List<String> commandList = new ArrayList<>();
        commandList.add(command);
        commandList.add(userInput);
        
        try {
            ProcessBuilder pb = new ProcessBuilder(commandList);
            pb.redirectErrorStream(true);
            Process process = pb.start();
            
            StringBuilder output = new StringBuilder();
            try (BufferedReader reader = new BufferedReader(
                    new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append(System.lineSeparator());
                }
            }
            
            int exitCode = process.waitFor();
            if (exitCode != 0) {
                return "Error: Command failed with exit code " + exitCode;
            }
            
            return output.toString().trim();
        } catch (Exception e) {
            return "Error: " + e.getClass().getSimpleName();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test 1 - Valid echo command:");
        System.out.println(executeCommand("echo", "test123"));
        
        System.out.println("\\nTest 2 - Valid input with dots:");
        System.out.println(executeCommand("echo", "file.txt"));
        
        System.out.println("\\nTest 3 - Invalid command injection attempt:");
        System.out.println(executeCommand("echo", "test;ls"));
        
        System.out.println("\\nTest 4 - Command not in whitelist:");
        System.out.println(executeCommand("rm", "file.txt"));
        
        System.out.println("\\nTest 5 - Input too long:");
        System.out.println(executeCommand("echo", "a".repeat(101)));
    }
}
