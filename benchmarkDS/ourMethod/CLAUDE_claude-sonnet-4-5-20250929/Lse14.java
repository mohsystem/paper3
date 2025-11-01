
import java.util.Scanner;
import java.util.InputMismatchException;

public class Lse14 {
    // Predefined array with fixed bounds
    private static final int[] id_sequence = {100, 200, 300, 400, 500};
    
    /**
     * Retrieves value at specified index from id_sequence array.
     * Validates index to prevent out-of-bounds access.
     * 
     * @param index The index to retrieve from id_sequence
     * @return The value at the specified index, or -1 if invalid
     */
    public static int getValueAtIndex(int index) {
        // Input validation: Check bounds to prevent array access violations
        // Reject negative indices and indices beyond array length
        if (index < 0 || index >= id_sequence.length) {
            // Fail closed: return error indicator without exposing internal details
            System.err.println("Error: Invalid index");
            return -1;
        }
        
        // Bounds checked: safe to access array
        return id_sequence[index];
    }
    
    /**
     * Prompts user for index and returns corresponding value.
     * Validates all user input before processing.
     * 
     * @return The value at user-specified index, or -1 on error
     */
    public static int promptAndGetValue() {
        Scanner scanner = null;
        try {
            scanner = new Scanner(System.in);
            System.out.print("Enter index (0-" + (id_sequence.length - 1) + "): ");
            
            // Input validation: Check type - only accept integers
            if (!scanner.hasNextInt()) {
                System.err.println("Error: Invalid input type");
                return -1;
            }
            
            int index = scanner.nextInt();
            
            // Delegate to validated function
            return getValueAtIndex(index);
            
        } catch (InputMismatchException e) {
            // Exception handling: Catch type mismatches
            System.err.println("Error: Invalid input");
            return -1;
        } catch (Exception e) {
            // Catch-all for unexpected errors, no internal details leaked
            System.err.println("Error: Processing failed");
            return -1;
        } finally {
            // Resource cleanup: Close scanner if we created it
            if (scanner != null) {
                scanner.close();
            }
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid index at start
        System.out.println("Test 1 - Index 0: " + getValueAtIndex(0));
        
        // Test case 2: Valid index in middle
        System.out.println("Test 2 - Index 2: " + getValueAtIndex(2));
        
        // Test case 3: Valid index at end
        System.out.println("Test 3 - Index 4: " + getValueAtIndex(4));
        
        // Test case 4: Invalid negative index
        System.out.println("Test 4 - Index -1: " + getValueAtIndex(-1));
        
        // Test case 5: Invalid index beyond bounds
        System.out.println("Test 5 - Index 10: " + getValueAtIndex(10));
    }
}
