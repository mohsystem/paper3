
import java.io.*;

public class Lse110 {
    public static int executeIdCommand(String username) {
        if (username == null || username.trim().isEmpty()) {
            return -1;
        }
        
        // Sanitize username to prevent command injection
        if (!username.matches("^[a-zA-Z0-9_-]+$")) {
            System.err.println("Invalid username format");
            return -1;
        }
        
        try {
            ProcessBuilder processBuilder = new ProcessBuilder("id", username);
            Process process = processBuilder.start();
            
            // Read output
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            String line;
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
            }
            
            // Read error stream
            BufferedReader errorReader = new BufferedReader(
                new InputStreamReader(process.getErrorStream())
            );
            while ((line = errorReader.readLine()) != null) {
                System.err.println(line);
            }
            
            int exitStatus = process.waitFor();
            return exitStatus;
            
        } catch (IOException | InterruptedException e) {
            System.err.println("Error executing command: " + e.getMessage());
            return -1;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {"root", "nobody", "daemon", "invaliduser123", "admin"};
        
        if (args.length > 0) {
            // Use command line argument if provided
            int exitStatus = executeIdCommand(args[0]);
            System.out.println("Exit status: " + exitStatus);
        } else {
            // Run test cases
            System.out.println("Running test cases:\\n");
            for (String username : testCases) {
                System.out.println("Testing username: " + username);
                int exitStatus = executeIdCommand(username);
                System.out.println("Exit status: " + exitStatus);
                System.out.println("---");
            }
        }
    }
}
