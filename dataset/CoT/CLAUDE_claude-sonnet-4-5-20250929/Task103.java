
public class Task103 {
    public static boolean checkPasswordStrength(String password) {
        // Handle null or empty password securely
        if (password == null || password.isEmpty()) {
            return false;
        }
        
        // Check minimum length
        if (password.length() < 8) {
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
            
            // Early exit if all criteria met
            if (hasUppercase && hasLowercase && hasDigit) {
                return true;
            }
        }
        
        return hasUppercase && hasLowercase && hasDigit;
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testPasswords = {
            "Abcd1234",      // Valid - meets all criteria
            "abcd1234",      // Invalid - no uppercase
            "ABCD1234",      // Invalid - no lowercase
            "Abcdefgh",      // Invalid - no digit
            "Abc123"         // Invalid - too short
        };
        
        System.out.println("Password Strength Checker Results:");
        for (String password : testPasswords) {
            boolean isStrong = checkPasswordStrength(password);
            System.out.println("Password: " + password + " -> " + (isStrong ? "STRONG" : "WEAK"));
        }
    }
}
