
import java.util.regex.*;
import java.util.*;

public class Task109 {
    public static class ValidationResult {
        public boolean isValid;
        public String message;
        
        public ValidationResult(boolean isValid, String message) {
            this.isValid = isValid;
            this.message = message;
        }
    }
    
    public static ValidationResult validateEmail(String email) {
        String emailPattern = "^[a-zA-Z0-9_+&*-]+(?:\\\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$";
        Pattern pattern = Pattern.compile(emailPattern);
        Matcher matcher = pattern.matcher(email);
        
        if (matcher.matches()) {
            return new ValidationResult(true, "Valid email address");
        } else {
            return new ValidationResult(false, "Invalid email address");
        }
    }
    
    public static ValidationResult validatePhone(String phone) {
        String phonePattern = "^\\\\+?[1-9]\\\\d{1,14}$|^\\\\(?\\\\d{3}\\\\)?[-.\\\\s]?\\\\d{3}[-.\\\\s]?\\\\d{4}$";
        Pattern pattern = Pattern.compile(phonePattern);
        Matcher matcher = pattern.matcher(phone);
        
        if (matcher.matches()) {
            return new ValidationResult(true, "Valid phone number");
        } else {
            return new ValidationResult(false, "Invalid phone number");
        }
    }
    
    public static ValidationResult validatePassword(String password) {
        String passwordPattern = "^(?=.*[a-z])(?=.*[A-Z])(?=.*\\\\d)(?=.*[@$!%*?&])[A-Za-z\\\\d@$!%*?&]{8,}$";
        Pattern pattern = Pattern.compile(passwordPattern);
        Matcher matcher = pattern.matcher(password);
        
        if (matcher.matches()) {
            return new ValidationResult(true, "Valid password");
        } else {
            return new ValidationResult(false, "Invalid password (must be 8+ chars with uppercase, lowercase, digit, and special character)");
        }
    }
    
    public static ValidationResult validateUsername(String username) {
        String usernamePattern = "^[a-zA-Z0-9_]{3,20}$";
        Pattern pattern = Pattern.compile(usernamePattern);
        Matcher matcher = pattern.matcher(username);
        
        if (matcher.matches()) {
            return new ValidationResult(true, "Valid username");
        } else {
            return new ValidationResult(false, "Invalid username (must be 3-20 alphanumeric characters or underscore)");
        }
    }
    
    public static ValidationResult validateURL(String url) {
        String urlPattern = "^(https?|ftp)://[^\\\\s/$.?#].[^\\\\s]*$";
        Pattern pattern = Pattern.compile(urlPattern, Pattern.CASE_INSENSITIVE);
        Matcher matcher = pattern.matcher(url);
        
        if (matcher.matches()) {
            return new ValidationResult(true, "Valid URL");
        } else {
            return new ValidationResult(false, "Invalid URL");
        }
    }
    
    public static void main(String[] args) {
        System.out.println("===== Test Case 1: Email Validation =====");
        ValidationResult result1 = validateEmail("user@example.com");
        System.out.println("Input: user@example.com");
        System.out.println("Result: " + result1.isValid + " - " + result1.message);
        System.out.println();
        
        System.out.println("===== Test Case 2: Phone Validation =====");
        ValidationResult result2 = validatePhone("123-456-7890");
        System.out.println("Input: 123-456-7890");
        System.out.println("Result: " + result2.isValid + " - " + result2.message);
        System.out.println();
        
        System.out.println("===== Test Case 3: Password Validation =====");
        ValidationResult result3 = validatePassword("SecurePass123!");
        System.out.println("Input: SecurePass123!");
        System.out.println("Result: " + result3.isValid + " - " + result3.message);
        System.out.println();
        
        System.out.println("===== Test Case 4: Username Validation =====");
        ValidationResult result4 = validateUsername("john_doe123");
        System.out.println("Input: john_doe123");
        System.out.println("Result: " + result4.isValid + " - " + result4.message);
        System.out.println();
        
        System.out.println("===== Test Case 5: URL Validation =====");
        ValidationResult result5 = validateURL("https://www.example.com");
        System.out.println("Input: https://www.example.com");
        System.out.println("Result: " + result5.isValid + " - " + result5.message);
    }
}
