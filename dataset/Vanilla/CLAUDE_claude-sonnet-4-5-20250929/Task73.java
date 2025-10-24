
import java.util.regex.*;

public class Task73 {
    public static boolean isValidPassword(String password) {
        if (password == null || password.length() < 8) {
            return false;
        }
        
        boolean hasUpperCase = false;
        boolean hasLowerCase = false;
        boolean hasDigit = false;
        boolean hasSpecialChar = false;
        
        String specialChars = "!@#$%^&*()_+-=[]{}|;:,.<>?";
        
        for (char c : password.toCharArray()) {
            if (Character.isUpperCase(c)) {
                hasUpperCase = true;
            } else if (Character.isLowerCase(c)) {
                hasLowerCase = true;
            } else if (Character.isDigit(c)) {
                hasDigit = true;
            } else if (specialChars.indexOf(c) != -1) {
                hasSpecialChar = true;
            }
        }
        
        return hasUpperCase && hasLowerCase && hasDigit && hasSpecialChar;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: 'Admin@123' -> " + isValidPassword("Admin@123"));
        System.out.println("Test 2: 'password' -> " + isValidPassword("password"));
        System.out.println("Test 3: 'Pass123' -> " + isValidPassword("Pass123"));
        System.out.println("Test 4: 'ADMIN@123' -> " + isValidPassword("ADMIN@123"));
        System.out.println("Test 5: 'Secure#Pass2024' -> " + isValidPassword("Secure#Pass2024"));
    }
}
