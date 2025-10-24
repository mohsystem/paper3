import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task55 {

    /**
     * Validates an email address using a regular expression.
     *
     * @param email The email string to validate.
     * @return true if the email is valid, false otherwise.
     */
    public static boolean isValidEmail(String email) {
        if (email == null || email.isEmpty()) {
            return false;
        }
        // A common regex for email validation. It's not 100% RFC compliant but covers most common cases.
        String emailRegex = "^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$";
        Pattern pattern = Pattern.compile(emailRegex);
        Matcher matcher = pattern.matcher(email);
        return matcher.matches();
    }

    public static void main(String[] args) {
        String[] testEmails = {
            "test.user@example.com",      // Valid
            "user@sub.domain.co.uk",  // Valid
            "invalid-email",              // Invalid (no @)
            "user@.com",                  // Invalid (domain starts with a dot)
            "user@domain..com"            // Invalid (consecutive dots in domain)
        };

        System.out.println("--- Java Email Validation ---");
        for (String email : testEmails) {
            System.out.println("Email: " + email + " -> Is Valid: " + isValidEmail(email));
        }
    }
}