import java.util.regex.Pattern;

public class Task182 {

    private static final Pattern NON_DIGIT_PATTERN = Pattern.compile("[^0-9]");

    /**
     * Cleans a phone number by removing punctuation and country code.
     * Validates the number against NANP rules.
     *
     * @param input The raw phone number string.
     * @return A 10-digit clean phone number string.
     * @throws IllegalArgumentException if the number is invalid.
     */
    public static String cleanPhoneNumber(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input string cannot be null.");
        }

        String digits = NON_DIGIT_PATTERN.matcher(input).replaceAll("");

        if (digits.length() == 11) {
            if (digits.charAt(0) == '1') {
                digits = digits.substring(1);
            } else {
                throw new IllegalArgumentException("11-digit number must start with 1.");
            }
        }

        if (digits.length() != 10) {
            throw new IllegalArgumentException("Number must be 10 digits.");
        }

        char areaCodeFirstDigit = digits.charAt(0);
        char exchangeCodeFirstDigit = digits.charAt(3);

        if (areaCodeFirstDigit == '0' || areaCodeFirstDigit == '1') {
            throw new IllegalArgumentException("Area code cannot start with 0 or 1.");
        }
        if (exchangeCodeFirstDigit == '0' || exchangeCodeFirstDigit == '1') {
            throw new IllegalArgumentException("Exchange code cannot start with 0 or 1.");
        }

        return digits;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "+1 (613)-995-0253",
            "613-995-0253",
            "1 613 995 0253",
            "613.995.0253",
            "(223) 456-7890",
            // Invalid cases for testing robustness
            "1 (123) 456-7890", // Area code starts with 1
            "(223) 056-7890", // Exchange code starts with 0
            "2 (223) 456-7890", // Invalid 11-digit number
            "(223) 456-789",    // Too short
            "123456789012"      // Too long
        };

        for (String testCase : testCases) {
            System.out.print("Input: \"" + testCase + "\" -> ");
            try {
                String cleaned = cleanPhoneNumber(testCase);
                System.out.println("Output: " + cleaned);
            } catch (IllegalArgumentException e) {
                System.out.println("Error: " + e.getMessage());
            }
        }
    }
}