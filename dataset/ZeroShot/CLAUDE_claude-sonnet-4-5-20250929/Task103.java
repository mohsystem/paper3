
import java.util.regex.Pattern;

public class Task103 {
    public static String checkPasswordStrength(String password) {
        if (password == null) {
            return "Weak: Password is null";
        }
        
        boolean hasMinLength = password.length() >= 8;
        boolean hasUppercase = !password.equals(password.toLowerCase());
        boolean hasLowercase = !password.equals(password.toUpperCase());
        boolean hasDigit = password.matches(".*\\\\d.*");
        
        if (hasMinLength && hasUppercase && hasLowercase && hasDigit) {
            return "Strong";
        } else {
            StringBuilder weakness = new StringBuilder("Weak: ");
            if (!hasMinLength) weakness.append("Less than 8 characters. ");
            if (!hasUppercase) weakness.append("No uppercase letter. ");
            if (!hasLowercase) weakness.append("No lowercase letter. ");
            if (!hasDigit) weakness.append("No digit. ");
            return weakness.toString().trim();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + checkPasswordStrength("Pass123word"));
        System.out.println("Test 2: " + checkPasswordStrength("weak"));
        System.out.println("Test 3: " + checkPasswordStrength("NoDigits"));
        System.out.println("Test 4: " + checkPasswordStrength("alllowercase123"));
        System.out.println("Test 5: " + checkPasswordStrength("ALLUPPERCASE123"));
    }
}
