import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task55 {

    /**
     * A pre-compiled regex pattern for email validation.
     * This regex is based on OWASP recommendations for a practical, common-use validation.
     * It is not fully RFC 5322 compliant but covers most standard email formats.
     */
    private static final String EMAIL_REGEX =
        "^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$";

    private static final Pattern EMAIL_PATTERN = Pattern.compile(EMAIL_REGEX);

    /**
     * Validates an email address against a standard pattern.
     *
     * @param email The string to be validated.
     * @return true if the email is valid, false otherwise.
     */
    public static boolean isValidEmail(String email) {
        if (email == null) {
            return false;
        }
        Matcher matcher = EMAIL_PATTERN.matcher(email);
        return matcher.matches();
    }

    public static void main(String[] args) {
        String[] testEmails = {
            "test@example.com",        // valid
            "test.name@example.co.uk", // valid
            "test..name@example.com",  // invalid (consecutive dots)
            "test@example",            // invalid (no TLD)
            "@example.com"             // invalid (empty local part)
        };

        for (String email : testEmails) {
            System.out.println("Email: " + email + " is valid? " + isValidEmail(email));
        }
    }
}