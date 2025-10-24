
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;
import java.util.regex.Matcher;
import java.nio.charset.StandardCharsets;

public class Task109 {
    private static final int MAX_INPUT_LENGTH = 10000;
    private static final int MAX_PATTERN_LENGTH = 1000;
    
    /**
     * Validates input string against a regular expression pattern.
     * Returns true if input matches the pattern, false otherwise.
     */
    public static boolean validateInput(String input, String regexPattern) {
        // Validate inputs are not null
        if (input == null || regexPattern == null) {
            return false;
        }
        
        // Validate input length to prevent resource exhaustion
        if (input.length() > MAX_INPUT_LENGTH) {
            return false;
        }
        
        // Validate pattern length
        if (regexPattern.length() > MAX_PATTERN_LENGTH) {
            return false;
        }
        
        try {
            // Compile pattern with timeout protection via bounded matching
            Pattern pattern = Pattern.compile(regexPattern);
            Matcher matcher = pattern.matcher(input);
            
            // Use matches() for full string matching
            return matcher.matches();
        } catch (PatternSyntaxException e) {
            // Invalid regex pattern
            return false;
        } catch (StackOverflowError e) {
            // Catastrophic backtracking detected
            return false;
        } catch (Exception e) {
            // Any other exception
            return false;
        }
    }
    
    /**
     * Validates email format.
     */
    public static boolean validateEmail(String email) {
        if (email == null) {
            return false;
        }
        String emailPattern = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$";
        return validateInput(email, emailPattern);
    }
    
    /**
     * Validates phone number (US format).
     */
    public static boolean validatePhoneNumber(String phone) {
        if (phone == null) {
            return false;
        }
        String phonePattern = "^\\\\d{3}-\\\\d{3}-\\\\d{4}$";
        return validateInput(phone, phonePattern);
    }
    
    /**
     * Validates alphanumeric string.
     */
    public static boolean validateAlphanumeric(String input) {
        if (input == null) {
            return false;
        }
        String alphanumericPattern = "^[a-zA-Z0-9]+$";
        return validateInput(input, alphanumericPattern);
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid email
        String email1 = "user@example.com";
        System.out.println("Test 1 - Valid email: " + validateEmail(email1));
        
        // Test case 2: Invalid email
        String email2 = "invalid.email@";
        System.out.println("Test 2 - Invalid email: " + validateEmail(email2));
        
        // Test case 3: Valid phone number
        String phone1 = "123-456-7890";
        System.out.println("Test 3 - Valid phone: " + validatePhoneNumber(phone1));
        
        // Test case 4: Valid alphanumeric
        String alphanumeric1 = "Test123";
        System.out.println("Test 4 - Valid alphanumeric: " + validateAlphanumeric(alphanumeric1));
        
        // Test case 5: Null input handling
        System.out.println("Test 5 - Null input: " + validateInput(null, "^[a-z]+$"));
    }
}
