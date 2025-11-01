
import java.util.Scanner;

public class Lse18 {
    // Predefined array for demonstration
    private static final int[] DATA_ARRAY = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    /**
     * Returns the value at the specified index in the array.
     * Validates input to prevent out-of-bounds access.
     * 
     * @param index The index to retrieve from the array
     * @return The value at the specified index, or -1 if invalid
     */
    public static int getValueAtIndex(int index) {
        // Input validation: Check if index is within valid bounds
        // This prevents array index out of bounds exceptions and invalid memory access
        if (index < 0 || index >= DATA_ARRAY.length) {
            // Fail closed: return error indicator without exposing internal details
            return -1;
        }
        
        // Safe array access after validation
        return DATA_ARRAY[index];
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid index at beginning
        int result1 = getValueAtIndex(0);
        System.out.println("Test 1 - Index 0: " + result1);
        
        // Test case 2: Valid index in middle
        int result2 = getValueAtIndex(5);
        System.out.println("Test 2 - Index 5: " + result2);
        
        // Test case 3: Valid index at end
        int result3 = getValueAtIndex(9);
        System.out.println("Test 3 - Index 9: " + result3);
        
        // Test case 4: Invalid negative index
        int result4 = getValueAtIndex(-1);
        System.out.println("Test 4 - Index -1: " + result4);
        
        // Test case 5: Invalid index beyond array bounds
        int result5 = getValueAtIndex(15);
        System.out.println("Test 5 - Index 15: " + result5);
    }
}
