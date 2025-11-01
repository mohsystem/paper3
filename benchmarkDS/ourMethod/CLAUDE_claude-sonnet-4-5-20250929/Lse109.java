
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

public class Lse109 {
    // Security: Read /etc/passwd directly instead of executing shell commands
    // This prevents command injection (CWE-78) by avoiding system() calls
    public static String getUserInfo(String username) {
        if (username == null || username.isEmpty()) {
            return "Error: Username cannot be empty";
        }
        
        // Security: Validate username to contain only safe characters
        // Usernames should only contain alphanumeric, underscore, hyphen, and period
        if (!username.matches("^[a-zA-Z0-9._-]+$")) {
            return "Error: Invalid username format";
        }
        
        // Security: Limit username length to prevent resource exhaustion
        if (username.length() > 32) {
            return "Error: Username too long";
        }
        
        Path passwdPath = Paths.get("/etc/passwd");
        
        // Security: Validate that /etc/passwd is a regular file, not a symlink
        // This prevents TOCTOU and symlink attacks
        try {
            if (!Files.isRegularFile(passwdPath, LinkOption.NOFOLLOW_LINKS)) {
                return "Error: /etc/passwd is not a regular file";
            }
        } catch (Exception e) {
            return "Error: Cannot access /etc/passwd";
        }
        
        StringBuilder result = new StringBuilder();
        
        // Security: Use try-with-resources for automatic resource management
        // Read file directly instead of executing shell commands
        try (BufferedReader reader = Files.newBufferedReader(passwdPath, StandardCharsets.UTF_8)) {
            String line;
            boolean found = false;
            
            // Security: Limit lines read to prevent DoS
            int maxLines = 10000;
            int lineCount = 0;
            
            while ((line = reader.readLine()) != null && lineCount < maxLines) {
                lineCount++;
                
                // Security: Validate line length to prevent memory issues
                if (line.length() > 1024) {
                    continue;
                }
                
                // Parse line format: username:password:uid:gid:gecos:home:shell
                String[] fields = line.split(":", 2);
                if (fields.length > 0 && fields[0].equals(username)) {
                    result.append(line).append("\\n");
                    found = true;
                }
            }
            
            if (!found) {
                return "User not found: " + username;
            }
            
        } catch (IOException e) {
            return "Error: Cannot read /etc/passwd";
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testUsers = {"root", "daemon", "bin", "nonexistent", "test.user"};
        
        System.out.println("=== User Information Lookup ===\\n");
        
        for (String user : testUsers) {
            System.out.println("Looking up user: " + user);
            String info = getUserInfo(user);
            System.out.println(info);
            System.out.println("---");
        }
        
        // If command line argument provided, use it
        if (args.length > 0) {
            System.out.println("\\nCommand line lookup: " + args[0]);
            System.out.println(getUserInfo(args[0]));
        }
    }
}
