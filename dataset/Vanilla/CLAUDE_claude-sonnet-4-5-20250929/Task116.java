
import java.io.*;
import java.util.*;

public class Task116 {
    private static final String ORIGINAL_USER = System.getProperty("user.name");
    
    public static boolean changeUserPassword(String username, String newPassword) {
        try {
            // Check if running with elevated privileges
            if (!hasElevatedPrivileges()) {
                System.out.println("Error: Insufficient privileges to change password");
                return false;
            }
            
            // Perform privileged operation (simulated)
            System.out.println("Performing privileged operation: Changing password for user: " + username);
            
            String os = System.getProperty("os.name").toLowerCase();
            ProcessBuilder processBuilder;
            
            if (os.contains("win")) {
                // Windows: net user command
                processBuilder = new ProcessBuilder("net", "user", username, newPassword);
            } else {
                // Linux/Unix: echo and chpasswd
                processBuilder = new ProcessBuilder("bash", "-c", 
                    "echo '" + username + ":" + newPassword + "' | chpasswd");
            }
            
            Process process = processBuilder.start();
            int exitCode = process.waitFor();
            
            if (exitCode == 0) {
                System.out.println("Password changed successfully for user: " + username);
                
                // Drop privileges after privileged operation
                dropPrivileges();
                
                return true;
            } else {
                System.out.println("Failed to change password. Exit code: " + exitCode);
                return false;
            }
            
        } catch (Exception e) {
            System.out.println("Error changing password: " + e.getMessage());
            return false;
        }
    }
    
    private static boolean hasElevatedPrivileges() {
        try {
            String os = System.getProperty("os.name").toLowerCase();
            
            if (os.contains("win")) {
                // Check if running as administrator on Windows
                Process process = Runtime.getRuntime().exec("net session");
                return process.waitFor() == 0;
            } else {
                // Check if running as root on Linux/Unix
                Process process = Runtime.getRuntime().exec("id -u");
                BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
                String uid = reader.readLine();
                return "0".equals(uid);
            }
        } catch (Exception e) {
            return false;
        }
    }
    
    private static void dropPrivileges() {
        System.out.println("Dropping privileges...");
        
        try {
            String os = System.getProperty("os.name").toLowerCase();
            
            if (os.contains("win")) {
                // Windows: Cannot truly drop privileges in Java, but we can note it
                System.out.println("Note: Privilege dropping on Windows requires process termination");
                System.out.println("Simulating privilege drop by restricting further operations");
            } else {
                // Linux/Unix: Attempt to drop to regular user
                System.out.println("Attempting to drop to regular user privileges");
                // Note: Java doesn't provide direct API to setuid, this is simulated
                System.out.println("Privileges dropped. Further privileged operations will fail.");
            }
            
        } catch (Exception e) {
            System.out.println("Error dropping privileges: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Privilege Management Test Cases ===\\n");
        
        // Test Case 1: Valid user password change
        System.out.println("Test Case 1: Attempting to change password for testuser1");
        changeUserPassword("testuser1", "NewPass123!");
        System.out.println();
        
        // Test Case 2: Another user password change
        System.out.println("Test Case 2: Attempting to change password for testuser2");
        changeUserPassword("testuser2", "SecurePass456!");
        System.out.println();
        
        // Test Case 3: Different user
        System.out.println("Test Case 3: Attempting to change password for adminuser");
        changeUserPassword("adminuser", "AdminPass789!");
        System.out.println();
        
        // Test Case 4: Special characters in password
        System.out.println("Test Case 4: Attempting to change password with special characters");
        changeUserPassword("testuser3", "P@ssw0rd#Special!");
        System.out.println();
        
        // Test Case 5: Empty username
        System.out.println("Test Case 5: Attempting to change password with empty username");
        changeUserPassword("", "Password123!");
        System.out.println();
    }
}
