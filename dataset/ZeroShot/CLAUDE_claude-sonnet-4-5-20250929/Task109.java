
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task109 {
    // Validate email address
    public static boolean validateEmail(String email) {
        if (email == null || email.isEmpty()) {
            return false;
        }
        String emailRegex = "^[a-zA-Z0-9_+&*-]+(?:\\\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$";
        Pattern pattern = Pattern.compile(emailRegex);
        Matcher matcher = pattern.matcher(email);
        return matcher.matches();
    }
    
    // Validate phone number (US format)
    public static boolean validatePhone(String phone) {
        if (phone == null || phone.isEmpty()) {
            return false;
        }
        String phoneRegex = "^(\\\\+1)?[-.\\\\s]?\\\\(?([0-9]{3})\\\\)?[-.\\\\s]?([0-9]{3})[-.\\\\s]?([0-9]{4})$";
        Pattern pattern = Pattern.compile(phoneRegex);
        Matcher matcher = pattern.matcher(phone);
        return matcher.matches();
    }
    
    // Validate password (min 8 chars, at least 1 uppercase, 1 lowercase, 1 digit, 1 special char)
    public static boolean validatePassword(String password) {
        if (password == null || password.isEmpty()) {
            return false;
        }
        String passwordRegex = "^(?=.*[a-z])(?=.*[A-Z])(?=.*\\\\d)(?=.*[@$!%*?&])[A-Za-z\\\\d@$!%*?&]{8,}$";
        Pattern pattern = Pattern.compile(passwordRegex);
        Matcher matcher = pattern.matcher(password);
        return matcher.matches();
    }
    
    // Validate URL
    public static boolean validateURL(String url) {
        if (url == null || url.isEmpty()) {
            return false;
        }
        String urlRegex = "^(https?|ftp)://[^\\\\s/$.?#].[^\\\\s]*$";
        Pattern pattern = Pattern.compile(urlRegex, Pattern.CASE_INSENSITIVE);
        Matcher matcher = pattern.matcher(url);
        return matcher.matches();
    }
    
    // Validate username (alphanumeric and underscore, 3-16 chars)
    public static boolean validateUsername(String username) {
        if (username == null || username.isEmpty()) {
            return false;
        }
        String usernameRegex = "^[a-zA-Z0-9_]{3,16}$";
        Pattern pattern = Pattern.compile(usernameRegex);
        Matcher matcher = pattern.matcher(username);
        return matcher.matches();
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Email Validation ===");
        String email1 = "user@example.com";
        String email2 = "invalid.email@";
        System.out.println(email1 + " is valid: " + validateEmail(email1));
        System.out.println(email2 + " is valid: " + validateEmail(email2));
        
        System.out.println("\\n=== Test Case 2: Phone Validation ===");
        String phone1 = "(123) 456-7890";
        String phone2 = "123-456-7890";
        System.out.println(phone1 + " is valid: " + validatePhone(phone1));
        System.out.println(phone2 + " is valid: " + validatePhone(phone2));
        
        System.out.println("\\n=== Test Case 3: Password Validation ===");
        String pwd1 = "SecurePass123!";
        String pwd2 = "weak";
        System.out.println(pwd1 + " is valid: " + validatePassword(pwd1));
        System.out.println(pwd2 + " is valid: " + validatePassword(pwd2));
        
        System.out.println("\\n=== Test Case 4: URL Validation ===");
        String url1 = "https://www.example.com";
        String url2 = "not a url";
        System.out.println(url1 + " is valid: " + validateURL(url1));
        System.out.println(url2 + " is valid: " + validateURL(url2));
        
        System.out.println("\\n=== Test Case 5: Username Validation ===");
        String user1 = "john_doe123";
        String user2 = "ab";
        System.out.println(user1 + " is valid: " + validateUsername(user1));
        System.out.println(user2 + " is valid: " + validateUsername(user2));
    }
}
