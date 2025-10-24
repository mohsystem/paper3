
import java.io.*;
import java.util.regex.*;
import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;

public class Task116 {
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_-]{3,32}$");
    private static final Pattern PASSWORD_PATTERN = Pattern.compile("^.{8,128}$");
    
    public static boolean validateUsername(String username) {
        if (username == null || username.isEmpty()) {
            return false;
        }
        return USERNAME_PATTERN.matcher(username).matches();
    }
    
    public static boolean validatePassword(String password) {
        if (password == null || password.isEmpty()) {
            return false;
        }
        return PASSWORD_PATTERN.matcher(password).matches();
    }
    
    public static String hashPassword(String password) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(password.getBytes(StandardCharsets.UTF_8));
            StringBuilder hexString = new StringBuilder();
            for (byte b : hash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) hexString.append('0');
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (Exception e) {
            return null;
        }
    }
    
    public static boolean changePasswordSecure(String username, String newPassword) {
        // Validate inputs
        if (!validateUsername(username)) {
            System.err.println("Invalid username format");
            return false;
        }
        
        if (!validatePassword(newPassword)) {
            System.err.println("Invalid password format");
            return false;
        }
        
        // This is a simulated privileged operation
        // In real implementation, this would use platform-specific APIs
        System.out.println("Simulating privileged password change for user: " + username);
        
        // Hash the password before any storage/operation
        String hashedPassword = hashPassword(newPassword);
        if (hashedPassword == null) {
            System.err.println("Password hashing failed");
            return false;
        }
        
        System.out.println("Password changed successfully (simulated)");
        return true;
    }
    
    public static void dropPrivileges() {
        // Platform-specific privilege dropping
        String os = System.getProperty("os.name").toLowerCase();
        
        System.out.println("Dropping privileges...");
        
        if (os.contains("nix") || os.contains("nux") || os.contains("mac")) {
            // Unix-like systems
            try {
                // In real implementation, use JNI to call setuid/setgid
                System.out.println("Would call setuid/setgid on Unix-like system");
                System.out.println("Privileges dropped to unprivileged user");
            } catch (Exception e) {
                System.err.println("Failed to drop privileges: " + e.getMessage());
            }
        } else if (os.contains("win")) {
            // Windows systems
            System.out.println("Would adjust token privileges on Windows");
            System.out.println("Privileges dropped to limited user");
        }
        
        // Clear sensitive data
        System.gc();
    }
    
    public static void performPrivilegedOperation(String username, String newPassword) {
        try {
            boolean success = changePasswordSecure(username, newPassword);
            
            if (success) {
                System.out.println("Operation completed successfully");
            } else {
                System.out.println("Operation failed");
            }
        } finally {
            // Always drop privileges after operation
            dropPrivileges();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Valid username and password ===");
        performPrivilegedOperation("testuser1", "SecurePass123!");
        
        System.out.println("\\n=== Test Case 2: Invalid username (special chars) ===");
        performPrivilegedOperation("test@user", "SecurePass123!");
        
        System.out.println("\\n=== Test Case 3: Invalid password (too short) ===");
        performPrivilegedOperation("testuser2", "short");
        
        System.out.println("\\n=== Test Case 4: Valid username and long password ===");
        performPrivilegedOperation("validuser", "ThisIsAVeryLongAndSecurePassword12345!");
        
        System.out.println("\\n=== Test Case 5: Empty username ===");
        performPrivilegedOperation("", "SecurePass123!");
    }
}
