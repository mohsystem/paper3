
import java.util.Scanner;
import java.nio.charset.StandardCharsets;

public class Task103 {
    /**
     * Validates password strength based on specific criteria.
     * 
     * @param password The password string to validate
     * @return true if password meets all criteria, false otherwise
     */
    public static boolean checkPasswordStrength(String password) {
        // Validate input is not null
        if (password == null) {
            return false;
        }
        
        // Check minimum length requirement (at least 8 characters)
        if (password.length() < 8) {
            return false;
        }
        
        // Check maximum length to prevent resource exhaustion
        if (password.length() > 128) {
            return false;
        }
        
        boolean hasUppercase = false;
        boolean hasLowercase = false;
        boolean hasDigit = false;
        
        // Iterate through each character to check criteria
        for (int i = 0; i < password.length(); i++) {
            char c = password.charAt(i);
            
            if (Character.isUpperCase(c)) {
                hasUppercase = true;
            } else if (Character.isLowerCase(c)) {
                hasLowercase = true;
            } else if (Character.isDigit(c)) {
                hasDigit = true;
            }
            
            // Early exit if all criteria are met
            if (hasUppercase && hasLowercase && hasDigit) {
                return true;
            }
        }
        
        // Return true only if all three criteria are met
        return hasUppercase && hasLowercase && hasDigit;
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testPasswords = {
            "Abcdefg1",      // Valid: 8 chars, upper, lower, digit
            "short1A",       // Valid: 7 chars but meets all criteria (actually valid with 8 chars)
            "alllowercase1", // Invalid: no uppercase
            "ALLUPPERCASE1", // Invalid: no lowercase
            "NoDigitsHere"   // Invalid: no digit
        };
        
        System.out.println("Password Strength Checker");
        System.out.println("=========================");
        
        for (String password : testPasswords) {
            boolean isStrong = checkPasswordStrength(password);
            System.out.println("Password: \\"" + password + "\\" - " + 
                             (isStrong ? "STRONG" : "WEAK"));
        }
        
        // Additional test with null
        System.out.println("Password: null - " + 
                         (checkPasswordStrength(null) ? "STRONG" : "WEAK"));
    }
}
