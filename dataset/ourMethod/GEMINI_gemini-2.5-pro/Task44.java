import java.time.Year;
import java.util.regex.Pattern;

public class Task44 {

    // Rule #1: Validate against a strict pattern for format, type, and length.
    // Name must contain only letters and spaces, and be between 2 and 50 characters.
    private static final Pattern NAME_PATTERN = Pattern.compile("^[a-zA-Z ]{2,50}$");
    private static final int MIN_AGE = 0;
    private static final int MAX_AGE = 120;

    /**
     * Processes user name and age string, performing validation and returning a result.
     *
     * @param name The user's name.
     * @param ageStr The user's age as a string.
     * @return A formatted string with a greeting and calculated birth year, or an error message.
     */
    public static String processUserInput(String name, String ageStr) {
        // Rule #1 & #6: Ensure all input is validated and treated as untrusted.
        if (name == null || !NAME_PATTERN.matcher(name).matches()) {
            return "Error: Invalid name. Name must be 2-50 letters and spaces.";
        }

        if (ageStr == null) {
            return "Error: Age must not be null.";
        }
        
        int age;
        try {
            // Rule #6: Use safe parsing.
            age = Integer.parseInt(ageStr);
            
            // Rule #1: Validate range.
            if (age < MIN_AGE || age > MAX_AGE) {
                return "Error: Invalid age. Age must be between " + MIN_AGE + " and " + MAX_AGE + ".";
            }
        } catch (NumberFormatException e) {
            // Rule #6: Fail closed on parsing error.
            return "Error: Invalid age format. Please enter a whole number.";
        }

        // Operation on validated data
        int currentYear = Year.now().getValue();
        int birthYear = currentYear - age;

        // Rule #6: Use safe formatting. String.format is safe from injection with primitive types.
        // The 'name' variable has been validated against a strict whitelist pattern.
        return String.format("Hello, %s! You are %d years old. You were likely born in %d.", name, age, birthYear);
    }

    /**
     * Main method with test cases to demonstrate input validation.
     */
    public static void main(String[] args) {
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Valid input
        System.out.println("Test 1 (Valid):");
        System.out.println("Input: name='Alice', age='30'");
        System.out.println("Output: " + processUserInput("Alice", "30"));
        System.out.println();

        // Test Case 2: Invalid name (too short)
        System.out.println("Test 2 (Invalid Name - Length):");
        System.out.println("Input: name='J', age='25'");
        System.out.println("Output: " + processUserInput("J", "25"));
        System.out.println();

        // Test Case 3: Invalid name (contains numbers)
        System.out.println("Test 3 (Invalid Name - Chars):");
        System.out.println("Input: name='Bob123', age='40'");
        System.out.println("Output: " + processUserInput("Bob123", "40"));
        System.out.println();

        // Test Case 4: Invalid age (not a number)
        System.out.println("Test 4 (Invalid Age - Format):");
        System.out.println("Input: name='Charlie', age='twenty'");
        System.out.println("Output: " + processUserInput("Charlie", "twenty"));
        System.out.println();

        // Test Case 5: Invalid age (out of range)
        System.out.println("Test 5 (Invalid Age - Range):");
        System.out.println("Input: name='David', age='150'");
        System.out.println("Output: " + processUserInput("David", "150"));
        System.out.println();
    }
}