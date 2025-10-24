import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task109 {

    /**
     * Validates an input string against a given regular expression.
     *
     * @param input        The string to validate.
     * @param regexPattern The regular expression pattern.
     * @return true if the input matches the pattern, false otherwise.
     */
    public static boolean validateInput(String input, String regexPattern) {
        if (input == null || regexPattern == null) {
            return false;
        }
        Pattern pattern = Pattern.compile(regexPattern);
        Matcher matcher = pattern.matcher(input);
        return matcher.matches();
    }

    public static void main(String[] args) {
        // A simple regex for email validation
        String emailRegex = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";

        String[] testCases = {
            "test@example.com",         // Valid
            "test.name@example.co.uk",  // Valid
            "test@.com",                // Invalid
            "test",                     // Invalid
            "test@example"              // Invalid
        };

        System.out.println("--- Testing Email Validation ---");
        for (String testCase : testCases) {
            boolean isValid = validateInput(testCase, emailRegex);
            System.out.println("Input: \"" + testCase + "\", Is Valid? " + isValid);
        }
    }
}