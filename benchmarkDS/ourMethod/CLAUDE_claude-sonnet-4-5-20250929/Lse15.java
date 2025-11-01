
import java.util.Scanner;

public class Lse15 {
    // Predefined array of id_sequence values
    private static final int[] ID_SEQUENCE = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    
    /**
     * Retrieves value at specified index from id_sequence array
     * @param index The index to retrieve (must be within array bounds)
     * @return The value at the specified index
     * @throws IllegalArgumentException if index is out of bounds
     */
    public static int getValueAtIndex(int index) {
        // Input validation: check if index is within valid bounds
        if (index < 0 || index >= ID_SEQUENCE.length) {
            throw new IllegalArgumentException("Invalid index: must be between 0 and " + (ID_SEQUENCE.length - 1));
        }
        
        // Return value at valid index
        return ID_SEQUENCE[index];
    }
    
    /**
     * Prompts user for index and returns corresponding value
     * @param scanner Scanner object for input
     * @return The value at user-specified index, or -1 on error
     */
    public static int promptAndGetValue(Scanner scanner) {
        try {
            System.out.print("Enter an index value (0-" + (ID_SEQUENCE.length - 1) + "): ");
            
            // Validate input is an integer
            if (!scanner.hasNextInt()) {
                System.err.println("Error: Invalid input type");
                scanner.next(); // Clear invalid input
                return -1;
            }
            
            int index = scanner.nextInt();
            return getValueAtIndex(index);
            
        } catch (IllegalArgumentException e) {
            // Handle out of bounds error without exposing internals
            System.err.println("Error: " + e.getMessage());
            return -1;
        } catch (Exception e) {
            // Catch any other exceptions
            System.err.println("Error: Invalid input");
            return -1;
        }
    }
    
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        
        // Test case 1: Valid index at start
        System.out.println("\\nTest case 1: Valid index 0");
        try {
            int result = getValueAtIndex(0);
            System.out.println("Value at index 0: " + result);
        } catch (Exception e) {
            System.err.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: Valid index in middle
        System.out.println("\\nTest case 2: Valid index 5");
        try {
            int result = getValueAtIndex(5);
            System.out.println("Value at index 5: " + result);
        } catch (Exception e) {
            System.err.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: Valid index at end
        System.out.println("\\nTest case 3: Valid index 9");
        try {
            int result = getValueAtIndex(9);
            System.out.println("Value at index 9: " + result);
        } catch (Exception e) {
            System.err.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test case 4: Invalid negative index
        System.out.println("\\nTest case 4: Invalid index -1");
        try {
            int result = getValueAtIndex(-1);
            System.out.println("Value at index -1: " + result);
        } catch (Exception e) {
            System.err.println("Test 4 failed (expected): " + e.getMessage());
        }
        
        // Test case 5: Invalid out of bounds index
        System.out.println("\\nTest case 5: Invalid index 15");
        try {
            int result = getValueAtIndex(15);
            System.out.println("Value at index 15: " + result);
        } catch (Exception e) {
            System.err.println("Test 5 failed (expected): " + e.getMessage());
        }
        
        scanner.close();
    }
}
