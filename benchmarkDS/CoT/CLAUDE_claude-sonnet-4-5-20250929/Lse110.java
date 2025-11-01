
import java.io.IOException;
import java.util.regex.Pattern;

public class Lse110 {
    // Username validation pattern: alphanumeric, dash, underscore only
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_-]+$");
    
    public static int getUserInfo(String username) {
        // Input validation to prevent command injection
        if (username == null || username.isEmpty()) {
            System.err.println("Error: Username cannot be empty");
            return -1;
        }
        
        if (!USERNAME_PATTERN.matcher(username).matches()) {
            System.err.println("Error: Invalid username format");
            return -1;
        }
        
        try {
            // Use ProcessBuilder for secure command execution
            ProcessBuilder processBuilder = new ProcessBuilder("id", username);
            processBuilder.redirectErrorStream(true);
            Process process = processBuilder.start();
            
            // Wait for process to complete and get exit status
            int exitStatus = process.waitFor();
            return exitStatus;
            
        } catch (IOException e) {
            System.err.println("Error executing command: " + e.getMessage());
            return -1;
        } catch (InterruptedException e) {
            System.err.println("Command interrupted: " + e.getMessage());
            Thread.currentThread().interrupt();
            return -1;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testUsers = {"root", "nobody", "invaliduser123", "test-user", "invalid;user"};
        
        if (args.length > 0) {
            // Use command line argument
            int exitStatus = getUserInfo(args[0]);
            System.out.println("Exit status: " + exitStatus);
        } else {
            // Run test cases
            System.out.println("Running test cases:");
            for (String user : testUsers) {
                System.out.println("\\nTesting user: " + user);
                int exitStatus = getUserInfo(user);
                System.out.println("Exit status: " + exitStatus);
            }
        }
    }
}
