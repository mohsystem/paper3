public class Task8 {

    /**
     * Finds the missing letter in an array of consecutive (increasing) letters.
     *
     * @param array An array of consecutive letters with one letter missing.
     * @return The missing letter.
     */
    public static char findMissingLetter(char[] array) {
        for (int i = 1; i < array.length; i++) {
            // Check if the current character is not the expected next character
            if (array[i] != array[i - 1] + 1) {
                // If not, the missing character is the one after the previous character
                return (char) (array[i - 1] + 1);
            }
        }
        // This part should be unreachable given the problem constraints
        // (always exactly one letter missing in an array of length >= 2).
        // A robust implementation might throw an exception here.
        return ' ';
    }

    public static void main(String[] args) {
        // Test Case 1
        char[] test1 = {'a', 'b', 'c', 'd', 'f'};
        System.out.println("Test 1: ['a','b','c','d','f'] -> " + findMissingLetter(test1));

        // Test Case 2
        char[] test2 = {'O', 'Q', 'R', 'S'};
        System.out.println("Test 2: ['O','Q','R','S'] -> " + findMissingLetter(test2));

        // Test Case 3
        char[] test3 = {'x', 'z'};
        System.out.println("Test 3: ['x', 'z'] -> " + findMissingLetter(test3));

        // Test Case 4
        char[] test4 = {'A', 'B', 'D'};
        System.out.println("Test 4: ['A', 'B', 'D'] -> " + findMissingLetter(test4));

        // Test Case 5
        char[] test5 = {'m', 'n', 'o', 'q', 'r'};
        System.out.println("Test 5: ['m', 'n', 'o', 'q', 'r'] -> " + findMissingLetter(test5));
    }
}