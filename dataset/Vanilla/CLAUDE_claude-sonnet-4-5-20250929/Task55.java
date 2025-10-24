
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task55 {
    public static boolean isValidEmail(String email) {
        if (email == null || email.isEmpty()) {
            return false;
        }
        
        // Regular expression for email validation
        String emailRegex = "^[a-zA-Z0-9_+&*-]+(?:\\\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$";
        Pattern pattern = Pattern.compile(emailRegex);
        Matcher matcher = pattern.matcher(email);
        
        return matcher.matches();
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testEmails = {
            "user@example.com",
            "john.doe@company.co.uk",
            "invalid.email@",
            "@invalid.com",
            "valid_email123@test-domain.org"
        };
        
        System.out.println("Email Validation Results:");
        for (String email : testEmails) {
            System.out.println(email + " : " + (isValidEmail(email) ? "Valid" : "Invalid"));
        }
    }
}
