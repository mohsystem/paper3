
// Java - Secure alternative using ProcessBuilder
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.Arrays;

public class Lse116 {
    // SECURE: Uses ProcessBuilder with separate command and arguments
    public static String executeListCommand(String directory) {
        StringBuilder output = new StringBuilder();
        try {
            // Validate directory path to prevent path traversal
            if (directory == null || directory.contains("..") || directory.contains(";") || directory.contains("|")) {
                return "Error: Invalid directory path";
            }
            
            // Use ProcessBuilder for safe command execution
            ProcessBuilder pb = new ProcessBuilder();
            
            // Platform-specific command
            if (System.getProperty("os.name").toLowerCase().contains("win")) {
                pb.command("cmd.exe", "/c", "dir", directory);
            } else {
                pb.command("ls", "-l", directory);
            }
            
            Process process = pb.start();
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            process.waitFor();
            reader.close();
            
        } catch (Exception e) {
            return "Error executing command: " + e.getMessage();
        }
        return output.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Current directory");
        System.out.println(executeListCommand("."));
        
        System.out.println("\\nTest Case 2: Root directory");
        System.out.println(executeListCommand("/"));
        
        System.out.println("\\nTest Case 3: Home directory");
        System.out.println(executeListCommand(System.getProperty("user.home")));
        
        System.out.println("\\nTest Case 4: Invalid path with traversal attempt");
        System.out.println(executeListCommand("../../../etc"));
        
        System.out.println("\\nTest Case 5: Command injection attempt");
        System.out.println(executeListCommand("; rm -rf /"));
    }
}
