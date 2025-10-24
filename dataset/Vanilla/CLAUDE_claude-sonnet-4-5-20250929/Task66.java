
import java.io.*;
import java.util.*;
import java.util.regex.*;

public class Task66 {
    
    public static String executeCommand(String command) {
        if (command == null || command.trim().isEmpty()) {
            return "Error: Command cannot be empty";
        }
        
        // Validate command format - basic security check
        if (!validateCommand(command)) {
            return "Error: Invalid or potentially dangerous command";
        }
        
        StringBuilder output = new StringBuilder();
        try {
            ProcessBuilder processBuilder = new ProcessBuilder();
            processBuilder.command("bash", "-c", command);
            processBuilder.redirectErrorStream(true);
            
            Process process = processBuilder.start();
            
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            int exitCode = process.waitFor();
            if (exitCode != 0 && output.length() == 0) {
                output.append("Command executed with exit code: ").append(exitCode);
            }
            
        } catch (IOException | InterruptedException e) {
            return "Error executing command: " + e.getMessage();
        }
        
        return output.toString().trim();
    }
    
    private static boolean validateCommand(String command) {
        // Basic validation - reject potentially dangerous commands
        String[] dangerousPatterns = {
            "rm\\\\s+-rf\\\\s+/",
            ":\\\\(\\\\)\\\\{.*:\\\\|:&\\\\}",  // fork bomb
            "dd\\\\s+if=",
            "mkfs",
            "format",
            ">(\\\\s)*/dev/sda"
        };
        
        for (String pattern : dangerousPatterns) {
            if (Pattern.compile(pattern).matcher(command).find()) {
                return false;
            }
        }
        
        return true;
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: echo 'Hello World'");
        System.out.println(executeCommand("echo 'Hello World'"));
        System.out.println();
        
        System.out.println("Test Case 2: ls -la | head -5");
        System.out.println(executeCommand("ls -la | head -5"));
        System.out.println();
        
        System.out.println("Test Case 3: date");
        System.out.println(executeCommand("date"));
        System.out.println();
        
        System.out.println("Test Case 4: pwd");
        System.out.println(executeCommand("pwd"));
        System.out.println();
        
        System.out.println("Test Case 5: echo $((5 + 3))");
        System.out.println(executeCommand("echo $((5 + 3))"));
        System.out.println();
    }
}
