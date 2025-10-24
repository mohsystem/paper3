
public class Task8 {
    public static char findMissingLetter(char[] array) {
        if (array == null || array.length < 2) {
            throw new IllegalArgumentException("Invalid input array");
        }
        
        for (int i = 0; i < array.length - 1; i++) {
            if (array[i + 1] - array[i] != 1) {
                return (char) (array[i] + 1);
            }
        }
        
        throw new IllegalArgumentException("No missing letter found");
    }
    
    public static void main(String[] args) {
        // Test case 1
        char[] test1 = {'a', 'b', 'c', 'd', 'f'};
        System.out.println("Test 1: " + findMissingLetter(test1)); // Expected: e
        
        // Test case 2
        char[] test2 = {'O', 'Q', 'R', 'S'};
        System.out.println("Test 2: " + findMissingLetter(test2)); // Expected: P
        
        // Test case 3
        char[] test3 = {'a', 'b', 'd'};
        System.out.println("Test 3: " + findMissingLetter(test3)); // Expected: c
        
        // Test case 4
        char[] test4 = {'m', 'n', 'o', 'q', 'r'};
        System.out.println("Test 4: " + findMissingLetter(test4)); // Expected: p
        
        // Test case 5
        char[] test5 = {'X', 'Z'};
        System.out.println("Test 5: " + findMissingLetter(test5)); // Expected: Y
    }
}
