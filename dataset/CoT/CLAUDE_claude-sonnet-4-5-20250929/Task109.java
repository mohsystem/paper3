
import java.util.Scanner;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task109 {
    
    // Validates email format
    public static boolean validateEmail(String email) {
        if (email == null || email.isEmpty()) {
            return false;
        }
        // RFC 5322 compliant email pattern with length limits
        if (email.length() > 254) {
            return false;
        }
        String emailPattern = "^[a-zA-Z0-9_+&*-]+(?:\\\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$";
        Pattern pattern = Pattern.compile(emailPattern);
        Matcher matcher = pattern.matcher(email);
        return matcher.matches();
    }
    
    // Validates phone number (US format)
    public static boolean validatePhoneNumber(String phone) {
        if (phone == null || phone.isEmpty()) {
            return false;
        }
        // US phone format: (123) 456-7890 or 123-456-7890 or 1234567890
        String phonePattern = "^(\\\\+?1)?[\\\\s-]?\\\\(?[0-9]{3}\\\\)?[\\\\s-]?[0-9]{3}[\\\\s-]?[0-9]{4}$";
        Pattern pattern = Pattern.compile(phonePattern);
        Matcher matcher = pattern.matcher(phone);
        return matcher.matches();
    }
    
    // Validates URL format
    public static boolean validateURL(String url) {
        if (url == null || url.isEmpty()) {
            return false;
        }
        if (url.length() > 2048) {
            return false;
        }
        String urlPattern = "^(https?://)(www\\\\.)?[-a-zA-Z0-9@:%._\\\\+~#=]{1,256}\\\\.[a-zA-Z0-9()]{1,6}\\\\b([-a-zA-Z0-9()@:%_\\\\+.~#?&//=]*)$";
        Pattern pattern = Pattern.compile(urlPattern);
        Matcher matcher = pattern.matcher(url);
        return matcher.matches();
    }
    
    // Validates strong password
    public static boolean validatePassword(String password) {
        if (password == null || password.isEmpty()) {
            return false;
        }
        // Password must be 8-128 characters, contain uppercase, lowercase, digit, and special character
        if (password.length() < 8 || password.length() > 128) {
            return false;
        }
        String passwordPattern = "^(?=.*[a-z])(?=.*[A-Z])(?=.*\\\\d)(?=.*[@$!%*?&])[A-Za-z\\\\d@$!%*?&]{8,}$";
        Pattern pattern = Pattern.compile(passwordPattern);
        Matcher matcher = pattern.matcher(password);
        return matcher.matches();
    }
    
    // Validates username (alphanumeric and underscore, 3-20 characters)
    public static boolean validateUsername(String username) {
        if (username == null || username.isEmpty()) {
            return false;
        }
        String usernamePattern = "^[a-zA-Z0-9_]{3,20}$";
        Pattern pattern = Pattern.compile(usernamePattern);
        Matcher matcher = pattern.matcher(username);
        return matcher.matches();
    }
    
    public static void main(String[] args) {
        System.out.println("=== User Input Validation Tests ===\\n");
        
        // Test Case 1: Valid inputs
        System.out.println("Test Case 1: Valid Inputs");
        System.out.println("Email 'user@example.com': " + validateEmail("user@example.com"));
        System.out.println("Phone '123-456-7890': " + validatePhoneNumber("123-456-7890"));
        System.out.println("URL 'https://www.example.com': " + validateURL("https://www.example.com"));
        System.out.println("Password 'SecurePass123!': " + validatePassword("SecurePass123!"));
        System.out.println("Username 'john_doe123': " + validateUsername("john_doe123"));
        System.out.println();
        
        // Test Case 2: Invalid email
        System.out.println("Test Case 2: Invalid Email");
        System.out.println("Email 'invalid.email': " + validateEmail("invalid.email"));
        System.out.println("Email 'user@': " + validateEmail("user@"));
        System.out.println("Email '@example.com': " + validateEmail("@example.com"));
        System.out.println();
        
        // Test Case 3: Invalid phone number
        System.out.println("Test Case 3: Invalid Phone Number");
        System.out.println("Phone '12345': " + validatePhoneNumber("12345"));
        System.out.println("Phone 'abc-def-ghij': " + validatePhoneNumber("abc-def-ghij"));
        System.out.println();
        
        // Test Case 4: Invalid password (weak)
        System.out.println("Test Case 4: Invalid Password");
        System.out.println("Password 'weak': " + validatePassword("weak"));
        System.out.println("Password 'NoSpecialChar1': " + validatePassword("NoSpecialChar1"));
        System.out.println("Password 'nouppercas1!': " + validatePassword("nouppercas1!"));
        System.out.println();
        
        // Test Case 5: Invalid username
        System.out.println("Test Case 5: Invalid Username");
        System.out.println("Username 'ab': " + validateUsername("ab"));
        System.out.println("Username 'user@name': " + validateUsername("user@name"));
        System.out.println("Username 'this_is_way_too_long_username': " + validateUsername("this_is_way_too_long_username"));
    }
}
