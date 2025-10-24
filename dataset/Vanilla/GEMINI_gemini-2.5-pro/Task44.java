import java.util.regex.Pattern;

public class Task44 {

    /**
     * Validates user input and returns a greeting message.
     *
     * @param name The user's name. Must be 2-50 chars, letters and spaces only.
     * @param age The user's age. Must be between 0 and 120.
     * @return A greeting string if the input is valid.
     * @throws IllegalArgumentException if the input is invalid.
     */
    public static String processUserInput(String name, int age) {
        // Validate name
        if (name == null || name.trim().isEmpty()) {
            throw new IllegalArgumentException("Name cannot be null or empty.");
        }
        if (name.length() < 2 || name.length() > 50) {
            throw new IllegalArgumentException("Name must be between 2 and 50 characters.");
        }
        // Regex to allow only letters and spaces
        if (!Pattern.matches("^[a-zA-Z\\s]+$", name)) {
            throw new IllegalArgumentException("Name can only contain letters and spaces.");
        }

        // Validate age
        if (age < 0 || age > 120) {
            throw new IllegalArgumentException("Age must be between 0 and 120.");
        }

        return "Hello, " + name.trim() + "! You are " + age + " years old.";
    }

    public static void main(String[] args) {
        // Test Cases
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Valid input
        try {
            System.out.println("Test Case 1 (Valid): " + processUserInput("Alice Smith", 30));
        } catch (IllegalArgumentException e) {
            System.out.println("Test Case 1 (Valid): FAILED - " + e.getMessage());
        }

        // Test Case 2: Invalid name (with numbers)
        try {
            System.out.println("Test Case 2 (Invalid Name): " + processUserInput("Bob123", 40));
        } catch (IllegalArgumentException e) {
            System.out.println("Test Case 2 (Invalid Name): PASSED - " + e.getMessage());
        }

        // Test Case 3: Invalid age (too high)
        try {
            System.out.println("Test Case 3 (Invalid Age): " + processUserInput("Charlie Brown", 150));
        } catch (IllegalArgumentException e) {
            System.out.println("Test Case 3 (Invalid Age): PASSED - " + e.getMessage());
        }

        // Test Case 4: Invalid name (empty)
        try {
            System.out.println("Test Case 4 (Empty Name): " + processUserInput("  ", 25));
        } catch (IllegalArgumentException e) {
            System.out.println("Test Case 4 (Empty Name): PASSED - " + e.getMessage());
        }

        // Test Case 5: Invalid name (special characters)
        try {
            System.out.println("Test Case 5 (Special Chars): " + processUserInput("Eve <script>", 22));
        } catch (IllegalArgumentException e) {
            System.out.println("Test Case 5 (Special Chars): PASSED - " + e.getMessage());
        }
        
        System.out.println("--- Test Cases Finished ---");
    }
}