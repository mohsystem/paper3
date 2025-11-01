
import java.util.Scanner;

public class Lse16 {
    // Predefined array with values
    private static final int[] VALUES = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    /**
     * Retrieves the value at the specified index from the array.
     * Validates input to prevent out-of-bounds access.
     * 
     * @param index The index to retrieve
     * @return The value at the specified index, or -1 if invalid
     */
    public static int getValueAtIndex(int index) {
        // Input validation: check if index is within valid range
        // This prevents array index out of bounds exception (CWE-129)
        if (index < 0 || index >= VALUES.length) {
            // Fail closed: return error indicator without exposing internal details
            System.err.println("Error: Invalid index");
            return -1;
        }
        
        // Safe array access after validation
        return VALUES[index];
    }
    
    public static void main(String[] args) {
        // Test cases with boundary and normal conditions
        System.out.println("Test Case 1 (index=0): " + getValueAtIndex(0));     // Expected: 10
        System.out.println("Test Case 2 (index=5): " + getValueAtIndex(5));     // Expected: 60
        System.out.println("Test Case 3 (index=9): " + getValueAtIndex(9));     // Expected: 100
        System.out.println("Test Case 4 (index=-1): " + getValueAtIndex(-1));   // Expected: -1 (error)
        System.out.println("Test Case 5 (index=10): " + getValueAtIndex(10));   // Expected: -1 (error)
    }
}
