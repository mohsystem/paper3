import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;

public class Task109 {

    /**
     * Validates an input string against a given regular expression.
     *
     * @param inputString The string to validate.
     * @param regexPattern The regular expression pattern.
     * @return true if the input string matches the entire pattern, false otherwise.
     * @apiNote It is assumed the regexPattern is from a trusted source to prevent ReDoS attacks.
     */
    public static boolean validateInput(String inputString, String regexPattern) {
        if (inputString == null || regexPattern == null) {
            return false;
        }
        try {
            return Pattern.matches(regexPattern, inputString);
        } catch (PatternSyntaxException e) {
            // Handle cases where the provided regex is invalid
            System.err.println("Invalid regex pattern: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Valid Email
        String emailValid = "test@example.com";
        String emailRegex = "^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$";
        System.out.println("1. Testing '" + emailValid + "' against email regex: " + (validateInput(emailValid, emailRegex) ? "Valid" : "Invalid"));

        // Test Case 2: Invalid Email
        String emailInvalid = "test@example";
        System.out.println("2. Testing '" + emailInvalid + "' against email regex: " + (validateInput(emailInvalid, emailRegex) ? "Valid" : "Invalid"));

        // Test Case 3: Valid US Phone Number
        String phoneValid = "123-456-7890";
        String phoneRegex = "^\\d{3}-\\d{3}-\\d{4}$";
        System.out.println("3. Testing '" + phoneValid + "' against phone regex: " + (validateInput(phoneValid, phoneRegex) ? "Valid" : "Invalid"));

        // Test Case 4: Valid Password (alphanumeric, 8+ characters)
        String passValid = "Password123";
        String passRegex = "^[A-Za-z0-9]{8,}$";
        System.out.println("4. Testing '" + passValid + "' against password regex: " + (validateInput(passValid, passRegex) ? "Valid" : "Invalid"));
        
        // Test Case 5: Invalid Password (too short)
        String passInvalid = "pass";
        System.out.println("5. Testing '" + passInvalid + "' against password regex: " + (validateInput(passInvalid, passRegex) ? "Valid" : "Invalid"));
    }
}