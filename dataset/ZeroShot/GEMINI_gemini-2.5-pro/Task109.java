import java.util.regex.Matcher;
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
            Pattern pattern = Pattern.compile(regexPattern);
            Matcher matcher = pattern.matcher(input);
            return matcher.matches();
        } catch (PatternSyntaxException e) {
            // Log the exception or handle it as appropriate
            System.err.println("Invalid regex pattern: " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        // Test Cases
        String[] inputs = {
            "test@example.com",
            "123-456-7890",
            "Alpha123",
            "Alpha123!",
            "not-an-email"
        };
        String[] patterns = {
            "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$", // Email
            "^\\d{3}-\\d{3}-\\d{4}$",                              // US Phone Number
            "^[a-zA-Z0-9]+$",                                     // Alphanumeric
            "^[a-zA-Z0-9]+$",                                     // Alphanumeric (fail case)
            "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"  // Email (fail case)
        };
        boolean[] expected = {
            true,
            true,
            true,
            false,
            false
        };

        System.out.println("--- Java Test Cases ---");
        for (int i = 0; i < inputs.length; i++) {
            boolean result = validateInput(inputs[i], patterns[i]);
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input: \"" + inputs[i] + "\"");
            System.out.println("Pattern: \"" + patterns[i] + "\"");
            System.out.println("Result: " + result + " | Expected: " + expected[i]);
            System.out.println("Status: " + (result == expected[i] ? "Passed" : "Failed"));
            System.out.println();
        }
    }
}