
import java.io.*;
import java.util.*;
import java.util.regex.Pattern;

public class Task66 {
    private static final Set<String> ALLOWED_COMMANDS = new HashSet<>(Arrays.asList(
        "ls", "pwd", "date", "whoami", "echo"
    ));
    
    private static final Pattern DANGEROUS_PATTERN = Pattern.compile(
        ".*[;&|`$(){}\\\\[\\\\]<>].*|.*\\\\.\\\\..*"
    );
    
    public static String executeCommand(String command) {
        if (command == null || command.trim().isEmpty()) {
            return "Error: Command cannot be empty";
        }
        
        command = command.trim();
        
        // Check for dangerous characters
        if (DANGEROUS_PATTERN.matcher(command).matches()) {
            return "Error: Command contains dangerous characters";
        }
        
        // Parse command
        String[] parts = command.split("\\\\s+");
        String baseCommand = parts[0];
        
        // Validate against whitelist
        if (!ALLOWED_COMMANDS.contains(baseCommand)) {
            return "Error: Command not allowed. Allowed commands: " + ALLOWED_COMMANDS;
        }
        
        // Validate arguments
        for (int i = 1; i < parts.length; i++) {
            if (parts[i].contains("/") || parts[i].contains("\\\\")) {\n                return "Error: Path traversal not allowed in arguments";\n            }\n        }\n        \n        // Execute command\n        try {\n            ProcessBuilder pb = new ProcessBuilder(parts);\n            pb.redirectErrorStream(true);\n            Process process = pb.start();\n            \n            BufferedReader reader = new BufferedReader(\n                new InputStreamReader(process.getInputStream())\n            );\n            \n            StringBuilder output = new StringBuilder();\n            String line;\n            while ((line = reader.readLine()) != null) {\n                output.append(line).append("\
");\n            }\n            \n            int exitCode = process.waitFor();\n            if (exitCode != 0) {\n                return "Error: Command exited with code " + exitCode + "\
" + output.toString();\n            }\n            \n            return output.toString().trim();\n            \n        } catch (IOException | InterruptedException e) {\n            return "Error executing command: " + e.getMessage();\n        }\n    }\n    \n    public static void main(String[] args) {\n        System.out.println("=== Test Case 1: Valid command 'pwd' ===");\n        System.out.println(executeCommand("pwd"));\n        System.out.println();\n        \n        System.out.println("=== Test Case 2: Valid command 'date' ===");\n        System.out.println(executeCommand("date"));\n        System.out.println();\n        \n        System.out.println("=== Test Case 3: Valid command 'echo' with arguments ===");\n        System.out.println(executeCommand("echo Hello World"));\n        System.out.println();\n        \n        System.out.println("=== Test Case 4: Invalid command with dangerous characters ===");\n        System.out.println(executeCommand("ls; rm -rf /"));\n        System.out.println();\n        \n        System.out.println("=== Test Case 5: Unauthorized command ===");\n        System.out.println(executeCommand("rm testfile.txt"));
        System.out.println();
    }
}
