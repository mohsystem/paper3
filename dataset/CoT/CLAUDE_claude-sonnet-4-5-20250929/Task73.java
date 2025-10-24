
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class Task73 {
    private static final int MIN_PASSWORD_LENGTH = 12;
    private static final Set<String> COMMON_PASSWORDS = new HashSet<>(Arrays.asList(
        "password123!", "Admin@123456", "Welcome@2024", "P@ssw0rd1234", "Administrator1!"
    ));
    
    public static boolean isValidAdminPassword(String password) {
        // Check for null or empty
        if (password == null || password.isEmpty()) {
            return false;
        }
        
        // Check minimum length
        if (password.length() < MIN_PASSWORD_LENGTH) {
            return false;
        }
        
        // Check for common passwords (case-insensitive)
        for (String common : COMMON_PASSWORDS) {
            if (password.equalsIgnoreCase(common)) {
                return false;
            }
        }
        
        // Check for required character types
        boolean hasUpper = false;
        boolean hasLower = false;
        boolean hasDigit = false;
        boolean hasSpecial = false;
        
        for (char c : password.toCharArray()) {
            if (Character.isUpperCase(c)) {
                hasUpper = true;
            } else if (Character.isLowerCase(c)) {
                hasLower = true;
            } else if (Character.isDigit(c)) {
                hasDigit = true;
            } else if (!Character.isWhitespace(c)) {
                hasSpecial = true;
            }
        }
        
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testPasswords = {
            "SecureP@ssw0rd123",  // Valid
            "Weak1!",              // Too short
            "nosymbolsupper1",     // No uppercase or special
            "NOLOWERCASE123!",     // No lowercase
            "Admin@123456"         // Common password
        };
        
        System.out.println("Admin Password Validation Results:");
        for (int i = 0; i < testPasswords.length; i++) {
            boolean result = isValidAdminPassword(testPasswords[i]);
            System.out.println("Test " + (i + 1) + ": \\"" + testPasswords[i] + "\\" -> " + 
                             (result ? "VALID" : "INVALID"));
        }
    }
}
