
public class Task103 {
    public static String checkPasswordStrength(String password) {
        if (password == null) {
            return "Weak";
        }
        
        boolean hasMinLength = password.length() >= 8;
        boolean hasUppercase = false;
        boolean hasLowercase = false;
        boolean hasDigit = false;
        
        for (int i = 0; i < password.length(); i++) {
            char c = password.charAt(i);
            if (Character.isUpperCase(c)) {
                hasUppercase = true;
            } else if (Character.isLowerCase(c)) {
                hasLowercase = true;
            } else if (Character.isDigit(c)) {
                hasDigit = true;
            }
        }
        
        if (hasMinLength && hasUppercase && hasLowercase && hasDigit) {
            return "Strong";
        } else {
            return "Weak";
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Strong password
        System.out.println("Test 1 - 'Password123': " + checkPasswordStrength("Password123"));
        
        // Test case 2: Too short
        System.out.println("Test 2 - 'Pass1': " + checkPasswordStrength("Pass1"));
        
        // Test case 3: No uppercase
        System.out.println("Test 3 - 'password123': " + checkPasswordStrength("password123"));
        
        // Test case 4: No digit
        System.out.println("Test 4 - 'PasswordTest': " + checkPasswordStrength("PasswordTest"));
        
        // Test case 5: Strong password with special characters
        System.out.println("Test 5 - 'SecurePass99!': " + checkPasswordStrength("SecurePass99!"));
    }
}
