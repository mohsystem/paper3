public class Task8 {

    /**
     * Finds the missing letter in an array of consecutive, increasing letters.
     * The array will always be valid and have exactly one letter missing.
     *
     * @param array An array of consecutive letters with one missing.
     * @return The missing letter.
     */
    public static char findMissingLetter(char[] array) {
        for (int i = 1; i < array.length; i++) {
            // Check if the current character's ASCII value is not one greater than the previous one.
            if (array[i] != array[i-1] + 1) {
                // If not, the missing character is the previous character + 1.
                return (char) (array[i-1] + 1);
            }
        }
        // This part is unreachable given the problem constraints (always one letter missing).
        // It is included to satisfy the Java compiler's requirement for a return statement.
        return ' ';
    }

    public static void main(String[] args) {
        // Test Case 1
        char[] test1 = {'a', 'b', 'c', 'd', 'f'};
        System.out.println("Test 1: ['a','b','c','d','f'] -> Expected: e, Got: " + findMissingLetter(test1));

        // Test Case 2
        char[] test2 = {'O', 'Q', 'R', 'S'};
        System.out.println("Test 2: ['O','Q','R','S'] -> Expected: P, Got: " + findMissingLetter(test2));

        // Test Case 3
        char[] test3 = {'x', 'z'};
        System.out.println("Test 3: ['x','z'] -> Expected: y, Got: " + findMissingLetter(test3));

        // Test Case 4
        char[] test4 = {'A', 'B', 'D'};
        System.out.println("Test 4: ['A','B','D'] -> Expected: C, Got: " + findMissingLetter(test4));

        // Test Case 5
        char[] test5 = {'m', 'n', 'o', 'q'};
        System.out.println("Test 5: ['m','n','o','q'] -> Expected: p, Got: " + findMissingLetter(test5));
    }
}