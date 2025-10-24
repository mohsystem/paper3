import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;

public class Task109 {

    /**
     * Validates a string against a given regular expression pattern.
     *
     * @param input The string to validate.
     * @param regexPattern The regular expression pattern.
     * @return true if the input matches the pattern, false otherwise.
     */
    public static boolean validateInput(String input, String regexPattern) {
        if (input == null || regexPattern == null) {
            return false;
        }
        try {
            return Pattern.matches(regexPattern, input);
        } catch (PatternSyntaxException e) {
            // Log the exception in a real application
            System.err.println("Invalid regex pattern: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        // Test cases
        String emailRegex = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,6}$";
        String phoneRegex = "^\\(?([0-9]{3})\\)?[-.\\s]?([0-9]{3})[-.\\s]?([0-9]{4})$";
        String usernameRegex = "^[a-zA-Z0-9]{5,15}$";

        // Test Case 1: Valid Email
        String validEmail = "test.user@example.com";
        boolean isEmailValid = validateInput(validEmail, emailRegex);
        System.out.println("1. Is '" + validEmail + "' a valid email? " + isEmailValid);

        // Test Case 2: Invalid Email
        String invalidEmail = "invalid-email@.com";
        boolean isEmailInvalid = validateInput(invalidEmail, emailRegex);
        System.out.println("2. Is '" + invalidEmail + "' a valid email? " + isEmailInvalid);

        // Test Case 3: Valid US Phone Number
        String validPhone = "(123) 456-7890";
        boolean isPhoneValid = validateInput(validPhone, phoneRegex);
        System.out.println("3. Is '" + validPhone + "' a valid US phone number? " + isPhoneValid);

        // Test Case 4: Valid Alphanumeric Username
        String validUsername = "user123";
        boolean isUsernameValid = validateInput(validUsername, usernameRegex);
        System.out.println("4. Is '" + validUsername + "' a valid username? " + isUsernameValid);

        // Test Case 5: Invalid Username (too short)
        String invalidUsername = "u1";
        boolean isUsernameInvalid = validateInput(invalidUsername, usernameRegex);
        System.out.println("5. Is '" + invalidUsername + "' a valid username? " + isUsernameInvalid);
    }
}