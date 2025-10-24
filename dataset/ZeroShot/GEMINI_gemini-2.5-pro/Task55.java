import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task55 {

    // Using a regex recommended by OWASP for email validation.
    // This regex is a good balance between correctness and preventing ReDoS attacks.
    private static final String EMAIL_REGEX =
            "^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@" +
            "(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$";

    private static final Pattern EMAIL_PATTERN = Pattern.compile(EMAIL_REGEX);

    public static boolean isValidEmail(String email) {
        if (email == null || email.isEmpty()) {
            return false;
        }
        // Basic length check to prevent very long inputs from bogging down the regex engine.
        if (email.length() > 254) {
            return false;
        }
        Matcher matcher = EMAIL_PATTERN.matcher(email);
        return matcher.matches();
    }

    public static void main(String[] args) {
        String[] testEmails = {
            "test@example.com",           // Valid
            "test.name@example.co.uk",  // Valid
            "test..name@example.com",   // Invalid (consecutive dots)
            "test@example",             // Invalid (no TLD)
            "plainaddress"              // Invalid (no '@' or domain)
        };

        System.out.println("Java Email Validation:");
        for (String email : testEmails) {
            System.out.println("'" + email + "' is valid: " + isValidEmail(email));
        }
    }
}