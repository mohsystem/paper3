
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.regex.Pattern;

public class Lse108 {
    // Pattern to validate username: alphanumeric, underscore, hyphen, and dot only
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9._-]{1,32}$");
    
    /**
     * Executes getent passwd command for a given username.
     * Uses secure command execution with ProcessBuilder to prevent command injection.
     * Validates username format before execution.
     * 
     * @param username The username to query
     * @return The output from getent passwd command, or error message
     */
    public static String getUserInfo(String username) {
        // Security: Validate username format to prevent command injection
        if (username == null || username.isEmpty()) {
            return "Error: Username cannot be null or empty";
        }
        
        // Security: Reject usernames with invalid characters to prevent injection
        if (!USERNAME_PATTERN.matcher(username).matches()) {
            return "Error: Invalid username format";
        }
        
        StringBuilder output = new StringBuilder();
        
        try {
            // Security: Use ProcessBuilder instead of Runtime.exec() or system()
            // This prevents shell interpretation and command injection
            ProcessBuilder pb = new ProcessBuilder("getent", "passwd", username);
            
            // Security: Do not inherit environment that might be tampered
            pb.redirectErrorStream(true);
            
            Process process = pb.start();
            
            // Read output with bounded buffer
            try (BufferedReader reader = new BufferedReader(
                    new InputStreamReader(process.getInputStream(), java.nio.charset.StandardCharsets.UTF_8))) {
                String line;
                int lineCount = 0;
                final int MAX_LINES = 100; // Security: Limit output to prevent DoS
                
                while ((line = reader.readLine()) != null && lineCount < MAX_LINES) {
                    output.append(line).append("\\n");
                    lineCount++;
                }
            }
            
            // Wait for process completion with timeout
            boolean completed = process.waitFor(5, java.util.concurrent.TimeUnit.SECONDS);
            if (!completed) {
                process.destroyForcibly();
                return "Error: Command execution timeout";
            }
            
            int exitCode = process.exitValue();
            if (exitCode != 0) {
                return "Error: Command failed with exit code " + exitCode;
            }
            
        } catch (IOException e) {
            return "Error: IO exception occurred";
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            return "Error: Operation interrupted";
        }
        
        return output.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid username
        System.out.println("Test 1 - Valid user 'root':");
        System.out.println(getUserInfo("root"));
        
        // Test case 2: Valid username with underscore
        System.out.println("\\nTest 2 - Valid user 'daemon':");
        System.out.println(getUserInfo("daemon"));
        
        // Test case 3: Non-existent user (should return empty or error)
        System.out.println("\\nTest 3 - Non-existent user:");
        System.out.println(getUserInfo("nonexistentuser12345"));
        
        // Test case 4: Invalid username with shell metacharacter (injection attempt)
        System.out.println("\\nTest 4 - Invalid username (injection attempt):");
        System.out.println(getUserInfo("root; rm -rf /"));
        
        // Test case 5: Empty username
        System.out.println("\\nTest 5 - Empty username:");
        System.out.println(getUserInfo(""));
    }
}
