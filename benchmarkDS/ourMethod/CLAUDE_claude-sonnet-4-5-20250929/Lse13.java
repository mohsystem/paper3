
import java.util.Scanner;
import java.util.InputMismatchException;

public class Lse13 {
    // Predefined array of ID sequences
    private static final int[] id_sequence = {100, 200, 300, 400, 500};
    
    /**
     * Returns the value at the specified index in the id_sequence array.
     * Validates index bounds before access to prevent out-of-bounds errors.
     * 
     * @param index The index to retrieve
     * @return The value at the specified index, or -1 if index is invalid
     */
    public static int getIdAtIndex(int index) {
        // Input validation: check if index is within valid bounds
        // This prevents array out-of-bounds access (CWE-125)
        if (index < 0 || index >= id_sequence.length) {
            // Fail closed: return error indicator without exposing internal details
            return -1;
        }
        
        // Safe array access after bounds validation
        return id_sequence[index];
    }
    
    public static void main(String[] args) {
        Scanner scanner = null;
        try {
            scanner = new Scanner(System.in);
            
            // Test cases
            System.out.println("Running test cases:");
            testCase(0, 100);
            testCase(2, 300);
            testCase(4, 500);
            testCase(-1, -1);  // Invalid: negative index
            testCase(10, -1);  // Invalid: index out of bounds
            
            System.out.println("\\nInteractive mode:");
            System.out.print("Enter an index value (0-" + (id_sequence.length - 1) + "): ");
            
            // Input validation: ensure input is an integer
            if (!scanner.hasNextInt()) {
                System.out.println("Error: Invalid input. Please enter a valid integer.");
                return;
            }
            
            int index = scanner.nextInt();
            
            int result = getIdAtIndex(index);
            
            // Check result and provide appropriate feedback
            if (result == -1) {
                // Generic error message - does not expose internal details
                System.out.println("Error: Invalid index.");
            } else {
                System.out.println("Value at index " + index + ": " + result);
            }
            
        } catch (InputMismatchException e) {
            // Handle input type mismatch without exposing stack traces
            System.out.println("Error: Invalid input type.");
        } catch (Exception e) {
            // Catch-all for unexpected errors - log securely without exposing details
            System.out.println("Error: An unexpected error occurred.");
        } finally {
            // Resource cleanup
            if (scanner != null) {
                scanner.close();
            }
        }
    }
    
    /**
     * Helper method to run test cases
     */
    private static void testCase(int index, int expected) {
        int result = getIdAtIndex(index);
        String status = (result == expected) ? "PASS" : "FAIL";
        System.out.println("Test: index=" + index + ", expected=" + expected + 
                         ", got=" + result + " - " + status);
    }
}
