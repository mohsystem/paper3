public class Task8 {

    public static char findMissingLetter(char[] array) {
        for (int i = 1; i < array.length; i++) {
            // If the difference between ASCII values is not 1, a letter is missing.
            if (array[i] - array[i-1] > 1) {
                // The missing letter is the previous letter's ASCII value + 1
                return (char) (array[i-1] + 1);
            }
        }
        // This part should not be reached given the problem constraints
        // (a letter is always missing).
        // It's here to make the compiler happy.
        return ' '; 
    }

    public static void main(String[] args) {
        // Test Case 1
        char[] test1 = {'a', 'b', 'c', 'd', 'f'};
        System.out.println("['a','b','c','d','f'] -> " + findMissingLetter(test1));

        // Test Case 2
        char[] test2 = {'O', 'Q', 'R', 'S'};
        System.out.println("['O','Q','R','S'] -> " + findMissingLetter(test2));

        // Test Case 3
        char[] test3 = {'x', 'z'};
        System.out.println("['x', 'z'] -> " + findMissingLetter(test3));

        // Test Case 4
        char[] test4 = {'F', 'G', 'H', 'J'};
        System.out.println("['F', 'G', 'H', 'J'] -> " + findMissingLetter(test4));

        // Test Case 5
        char[] test5 = {'m', 'n', 'o', 'q', 'r'};
        System.out.println("['m', 'n', 'o', 'q', 'r'] -> " + findMissingLetter(test5));
    }
}