import java.util.regex.Matcher;
import java.util.regex.Pattern;

class Task55 {
    /**
     * Validates an email address using a regular expression.
     *
     * @param email The string to validate.
     * @return true if the email is valid, false otherwise.
     */
    public static boolean isValidEmail(String email) {
        // A null or empty string is not a valid email
        if (email == null || email.isEmpty()) {
            return false;
        }

        // Regex for standard email validation.
        // ^                   # start of the string
        // [a-zA-Z0-9._%+-]+   # local part: one or more letters, numbers, or . _ % + -
        // @                   # a literal @
        // [a-zA-Z0-9.-]+      # domain name: one or more letters, numbers, or . -
        // \.                  # a literal .
        // [a-zA-Z]{2,}        # top-level domain: 2 or more letters
        // $                   # end of the string
        String emailRegex = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";

        Pattern pattern = Pattern.compile(emailRegex);
        Matcher matcher = pattern.matcher(email);

        return matcher.matches();
    }

    public static void main(String[] args) {
        String[] testEmails = {
            "test@example.com",          // Valid
            "test.name+alias@example.co.uk", // Valid
            "plainaddress",              // Invalid
            "@missing-local-part.com",   // Invalid
            "test@domain@domain.com"     // Invalid
        };

        System.out.println("Java Email Validation:");
        for (String email : testEmails) {
            System.out.println("'" + email + "' is valid: " + isValidEmail(email));
        }
    }
}