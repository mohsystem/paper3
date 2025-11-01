
import java.util.Scanner;
import java.util.InputMismatchException;

public class Lse17 {
    // Predefined array for demonstration
    private static final int[] VALUES = {10, 20, 30, 40, 50};
    
    /**
     * Retrieves the value at the specified index from the array.
     * Validates input to prevent out-of-bounds access.
     * 
     * @param index The index to retrieve
     * @return The value at the specified index, or -1 if index is invalid
     */
    public static int getValueAtIndex(int index) {
        // Input validation: Check if index is within valid bounds
        // This prevents array index out of bounds vulnerabilities
        if (index < 0 || index >= VALUES.length) {
            // Fail closed: return error indicator for invalid index
            return -1;
        }
        
        // Safe array access after bounds validation
        return VALUES[index];
    }
    
    public static void main(String[] args) {
        Scanner scanner = null;
        try {
            scanner = new Scanner(System.in);
            
            // Test cases
            System.out.println("Running test cases:");
            System.out.println("Test 1 (index=0): " + getValueAtIndex(0));
            System.out.println("Test 2 (index=2): " + getValueAtIndex(2));
            System.out.println("Test 3 (index=4): " + getValueAtIndex(4));
            System.out.println("Test 4 (index=-1): " + getValueAtIndex(-1));
            System.out.println("Test 5 (index=10): " + getValueAtIndex(10));
            
            // Interactive mode
            System.out.print("\\nEnter an index (0-" + (VALUES.length - 1) + "): ");
            
            // Input validation: check type to prevent InputMismatchException
            if (!scanner.hasNextInt()) {
                System.err.println("Error: Invalid input type. Expected integer.");
                return;
            }
            
            int userIndex = scanner.nextInt();
            
            int result = getValueAtIndex(userIndex);
            
            if (result == -1) {
                // Generic error message - do not leak internal details
                System.err.println("Error: Index out of valid range.");
            } else {
                System.out.println("Value at index " + userIndex + ": " + result);
            }
            
        } catch (InputMismatchException e) {
            // Exception handling: catch and provide generic error message
            System.err.println("Error: Invalid input format.");
        } catch (Exception e) {
            // Catch-all for unexpected errors - log generic message only
            System.err.println("Error: An unexpected error occurred.");
        } finally {
            // Resource cleanup: close scanner in finally block
            if (scanner != null) {
                scanner.close();
            }
        }
    }
}
