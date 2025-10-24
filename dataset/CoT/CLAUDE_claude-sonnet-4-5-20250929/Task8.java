
public class Task8 {
    public static char findMissingLetter(char[] array) {
        // Input validation
        if (array == null || array.length < 2) {
            throw new IllegalArgumentException("Invalid input array");
        }
        
        // Iterate through the array and find the missing letter
        for (int i = 0; i < array.length - 1; i++) {
            // Check if the next character is not consecutive
            if (array[i + 1] - array[i] != 1) {
                // Return the missing letter
                return (char) (array[i] + 1);
            }
        }
        
        // This should never happen with valid input
        throw new IllegalStateException("No missing letter found");
    }
    
    public static void main(String[] args) {
        // Test case 1
        char[] test1 = {'a', 'b', 'c', 'd', 'f'};
        System.out.println("Test 1: " + findMissingLetter(test1)); // Expected: e
        
        // Test case 2
        char[] test2 = {'O', 'Q', 'R', 'S'};
        System.out.println("Test 2: " + findMissingLetter(test2)); // Expected: P
        
        // Test case 3
        char[] test3 = {'m', 'n', 'p', 'q'};
        System.out.println("Test 3: " + findMissingLetter(test3)); // Expected: o
        
        // Test case 4
        char[] test4 = {'A', 'B', 'D', 'E'};
        System.out.println("Test 4: " + findMissingLetter(test4)); // Expected: C
        
        // Test case 5
        char[] test5 = {'x', 'z'};
        System.out.println("Test 5: " + findMissingLetter(test5)); // Expected: y
    }
}
