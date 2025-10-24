public class Task44 {

    /**
     * Validates that a string contains only alphanumeric characters.
     * @param s The string to validate.
     * @return true if the string is alphanumeric, false otherwise.
     */
    private static boolean isAlphanumeric(String s) {
        if (s == null) {
            return false;
        }
        return s.matches("^[a-zA-Z0-9]+$");
    }

    /**
     * Processes user input after validation.
     * @param username A string representing the username.
     * @param ageStr A string representing the user's age.
     * @return A string with the result of the operation or an error message.
     */
    public static String performOperation(String username, String ageStr) {
        // 1. Validate Username
        if (username == null || username.trim().isEmpty()) {
            return "Error: Username cannot be empty.";
        }
        if (username.length() < 3 || username.length() > 20) {
            return "Error: Username must be between 3 and 20 characters long.";
        }
        if (!isAlphanumeric(username)) {
            return "Error: Username must contain only alphanumeric characters.";
        }

        // 2. Validate Age
        int age;
        try {
            age = Integer.parseInt(ageStr);
        } catch (NumberFormatException e) {
            return "Error: Age must be a valid integer.";
        }

        if (age < 0 || age > 120) {
            return "Error: Age must be between 0 and 120.";
        }

        // 3. Perform Operation if all validations pass
        return "Success: Hello " + username + ", you are " + age + " years old.";
    }

    public static void main(String[] args) {
        // Main method with 5 test cases
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Valid input
        System.out.println("Test 1 (Valid):");
        System.out.println(performOperation("Alice", "30"));
        System.out.println();

        // Test Case 2: Invalid age (non-numeric)
        System.out.println("Test 2 (Invalid Age - Text):");
        System.out.println(performOperation("Bob", "twenty"));
        System.out.println();

        // Test Case 3: Invalid age (out of range)
        System.out.println("Test 3 (Invalid Age - Range):");
        System.out.println(performOperation("Charlie", "150"));
        System.out.println();

        // Test Case 4: Invalid username (special characters)
        System.out.println("Test 4 (Invalid Username - Chars):");
        System.out.println(performOperation("David!@", "40"));
        System.out.println();
        
        // Test Case 5: Invalid username (too short)
        System.out.println("Test 5 (Invalid Username - Length):");
        System.out.println(performOperation("Ed", "25"));
        System.out.println();
    }
}