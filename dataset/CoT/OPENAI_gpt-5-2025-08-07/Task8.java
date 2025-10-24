// Task8 - Java implementation
// Secure, single-responsibility function to find the missing letter.
// Based on a careful process: understand problem, validate inputs defensively, implement with safe arithmetic, and review for edge cases.
public class Task8 {
    // Returns the missing letter from a consecutive sequence containing exactly one missing element.
    public static char findMissingLetter(char[] array) {
        if (array == null || array.length < 2) {
            throw new IllegalArgumentException("Invalid input: array must have at least 2 characters.");
        }
        for (int i = 1; i < array.length; i++) {
            int diff = array[i] - array[i - 1];
            if (diff != 1) {
                return (char) (array[i - 1] + 1);
            }
        }
        // Should not happen with valid inputs per problem statement
        throw new IllegalStateException("No missing letter found.");
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        char[][] tests = new char[][] {
            {'a','b','c','d','f'},
            {'O','Q','R','S'},
            {'w','x','z'},
            {'A','B','C','E'},
            {'m','n','p','q','r'}
        };

        for (int i = 0; i < tests.length; i++) {
            char result = findMissingLetter(tests[i]);
            System.out.println("Test " + (i + 1) + " missing letter: " + result);
        }
    }
}