
import java.io.*;
import java.nio.file.*;
import java.util.stream.*;

public class Lse109 {
    // Secure method: Read and parse /etc/passwd directly without shell commands
    public static String getUserInfo(String username) {
        if (username == null || username.isEmpty()) {
            return "Error: Username cannot be empty";
        }
        
        // Validate username (alphanumeric, underscore, hyphen only)
        if (!username.matches("^[a-zA-Z0-9_-]+$")) {
            return "Error: Invalid username format";
        }
        
        try {
            // Directly read the file without shell execution
            Path passwdPath = Paths.get("/etc/passwd");
            if (!Files.exists(passwdPath)) {
                return "Error: /etc/passwd not found";
            }
            
            return Files.lines(passwdPath)
                .filter(line -> line.startsWith(username + ":"))
                .collect(Collectors.joining("\\n"));
                
        } catch (IOException e) {
            return "Error reading file: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testUsers = {"root", "daemon", "nobody", "testuser", "admin"};
        
        System.out.println("=== User Information Lookup (Secure) ===\\n");
        
        for (String user : testUsers) {
            System.out.println("Searching for user: " + user);
            String result = getUserInfo(user);
            System.out.println(result.isEmpty() ? "User not found" : result);
            System.out.println("---");
        }
    }
}
