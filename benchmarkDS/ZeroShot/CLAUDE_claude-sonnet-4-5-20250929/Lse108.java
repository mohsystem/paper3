
import java.io.*;
import java.util.Scanner;

public class Lse108 {
    public static String executeGetentPasswd(String username) {
        // Input validation to prevent command injection
        if (username == null || username.isEmpty()) {
            return "Error: Username cannot be null or empty";
        }
        
        // Sanitize username - only allow alphanumeric, underscore, and hyphen
        if (!username.matches("^[a-zA-Z0-9_-]+$")) {
            return "Error: Invalid username format";
        }
        
        StringBuilder output = new StringBuilder();
        
        try {
            // Use ProcessBuilder for safer command execution
            ProcessBuilder processBuilder = new ProcessBuilder("getent", "passwd", username);
            Process process = processBuilder.start();
            
            // Read the output
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            process.waitFor();
            reader.close();
            
        } catch (IOException | InterruptedException e) {
            return "Error: " + e.getMessage();
        }
        
        return output.toString().trim();
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testUsers = {"root", "nobody", "daemon", "testuser", "admin"};
        
        System.out.println("Testing getent passwd command:");
        System.out.println("================================\\n");
        
        for (String username : testUsers) {
            System.out.println("User: " + username);
            String result = executeGetentPasswd(username);
            System.out.println("Result: " + result);
            System.out.println("--------------------------------\\n");
        }
    }
}
