
import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;

public class Lse115 {
    // Secure method that lists directory contents without shell injection risks
    public static String listDirectory(String directoryPath) {
        try {
            // Validate and sanitize the directory path
            File directory = new File(directoryPath);
            
            // Check if directory exists and is a directory
            if (!directory.exists() || !directory.isDirectory()) {
                return "Invalid directory path";
            }
            
            // Use Java's built-in file listing instead of executing shell commands
            File[] files = directory.listFiles();
            if (files == null) {
                return "Cannot access directory";
            }
            
            StringBuilder result = new StringBuilder();
            for (File file : files) {
                result.append(file.getName()).append("\\n");
            }
            
            return result.toString();
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    // Alternative method using ProcessBuilder (safer than Runtime.exec)
    public static String executeListCommand(String directoryPath) {
        try {
            // Validate directory path
            File directory = new File(directoryPath);
            if (!directory.exists() || !directory.isDirectory()) {
                return "Invalid directory path";
            }
            
            // Use ProcessBuilder with separate arguments (prevents injection)
            ProcessBuilder processBuilder = new ProcessBuilder("ls", "-l", directoryPath);
            Process process = processBuilder.start();
            
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            
            StringBuilder output = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            process.waitFor();
            return output.toString();
        } catch (Exception e) {
            return "Error executing command: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Current directory");
        System.out.println(listDirectory("."));
        
        System.out.println("\\nTest Case 2: Home directory");
        System.out.println(listDirectory(System.getProperty("user.home")));
        
        System.out.println("\\nTest Case 3: Root directory");
        System.out.println(listDirectory("/"));
        
        System.out.println("\\nTest Case 4: Invalid directory");
        System.out.println(listDirectory("/nonexistent/path"));
        
        System.out.println("\\nTest Case 5: Temp directory");
        System.out.println(listDirectory(System.getProperty("java.io.tmpdir")));
    }
}
