
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task73 {
    public static boolean isValidAdminPassword(String password) {
        if (password == null || password.length() < 12) {
            return false;
        }
        
        // Check for at least one uppercase letter
        boolean hasUppercase = false;
        // Check for at least one lowercase letter
        boolean hasLowercase = false;
        // Check for at least one digit
        boolean hasDigit = false;
        // Check for at least one special character
        boolean hasSpecial = false;
        
        String specialChars = "!@#$%^&*()_+-=[]{}|;:,.<>?";
        
        for (char c : password.toCharArray()) {
            if (Character.isUpperCase(c)) {
                hasUppercase = true;
            } else if (Character.isLowerCase(c)) {
                hasLowercase = true;
            } else if (Character.isDigit(c)) {
                hasDigit = true;
            } else if (specialChars.indexOf(c) >= 0) {
                hasSpecial = true;
            }
        }
        
        // Check for common weak passwords
        String lowerPassword = password.toLowerCase();
        String[] weakPasswords = {"password", "admin", "123456", "qwerty", "letmein"};
        for (String weak : weakPasswords) {
            if (lowerPassword.contains(weak)) {
                return false;
            }
        }
        
        return hasUppercase && hasLowercase && hasDigit && hasSpecial;
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testPasswords = {
            "Admin@123456",           // Valid
            "SecureP@ssw0rd!",       // Valid
            "weak",                   // Invalid - too short
            "NoSpecialChar123",       // Invalid - no special character
            "Admin@Password2024"      // Valid
        };
        
        System.out.println("Admin Password Validation Results:");
        System.out.println("=====================================");
        for (String password : testPasswords) {
            boolean isValid = isValidAdminPassword(password);
            System.out.println("Password: " + password + " -> " + (isValid ? "VALID" : "INVALID"));
        }
    }
}
