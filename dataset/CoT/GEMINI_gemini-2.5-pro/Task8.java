public class Task8 {

    public static char findMissingLetter(char[] array) {
        for (int i = 1; i < array.length; i++) {
            if (array[i] != array[i-1] + 1) {
                return (char)(array[i-1] + 1);
            }
        }
        // This part should not be reachable given the problem constraints
        return ' '; 
    }

    public static void main(String[] args) {
        // Test Case 1
        char[] test1 = {'a', 'b', 'c', 'd', 'f'};
        System.out.println("Test Case 1: ['a','b','c','d','f'] -> " + findMissingLetter(test1));

        // Test Case 2
        char[] test2 = {'O', 'Q', 'R', 'S'};
        System.out.println("Test Case 2: ['O','Q','R','S'] -> " + findMissingLetter(test2));

        // Test Case 3
        char[] test3 = {'x', 'z'};
        System.out.println("Test Case 3: ['x','z'] -> " + findMissingLetter(test3));

        // Test Case 4
        char[] test4 = {'g', 'h', 'i', 'j', 'l'};
        System.out.println("Test Case 4: ['g','h','i','j','l'] -> " + findMissingLetter(test4));

        // Test Case 5
        char[] test5 = {'A', 'B', 'C', 'E'};
        System.out.println("Test Case 5: ['A','B','C','E'] -> " + findMissingLetter(test5));
    }
}