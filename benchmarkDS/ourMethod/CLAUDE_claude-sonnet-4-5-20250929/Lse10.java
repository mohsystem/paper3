
import java.util.Scanner;

public class Lse10 {
    // Function to get value from array with bounds checking
    // Returns the value at the given index if valid, otherwise returns -1
    private static int getValueFromArray(int[] array, int size, int index) {
        // Validate input parameters to prevent out-of-bounds access
        if (array == null) {
            System.err.println("Error: Array is null");
            return -1;
        }
        
        // Ensure index is within valid bounds [0, size-1]
        if (index < 0 || index >= size) {
            System.err.println("Error: Index out of bounds");
            return -1;
        }
        
        // Ensure size matches actual array length to prevent inconsistencies
        if (size > array.length) {
            System.err.println("Error: Size exceeds array length");
            return -1;
        }
        
        // Safe to access array at this point
        return array[index];
    }
    
    public static void main(String[] args) {
        // Predefined array for testing
        int[] id_sequence = {10, 20, 30, 40, 50};
        int size = id_sequence.length;
        
        // Use try-with-resources to ensure Scanner is properly closed
        try (Scanner scanner = new Scanner(System.in)) {
            // Validate that input is available
            if (!scanner.hasNextInt()) {
                System.err.println("Error: Invalid input");
                System.exit(1);
            }
            
            // Read index from standard input
            int index = scanner.nextInt();
            
            // Get value from array with bounds checking
            int result = getValueFromArray(id_sequence, size, index);
            
            // Exit with the retrieved value (or error code -1)
            System.exit(result);
            
        } catch (Exception e) {
            // Handle any unexpected exceptions
            System.err.println("Error: " + e.getMessage());
            System.exit(1);
        }
        
        // Test cases (uncomment to run)
        /*
        System.out.println("Test 1 (index 0): " + getValueFromArray(id_sequence, size, 0)); // Expected: 10
        System.out.println("Test 2 (index 2): " + getValueFromArray(id_sequence, size, 2)); // Expected: 30
        System.out.println("Test 3 (index 4): " + getValueFromArray(id_sequence, size, 4)); // Expected: 50
        System.out.println("Test 4 (index -1): " + getValueFromArray(id_sequence, size, -1)); // Expected: -1
        System.out.println("Test 5 (index 10): " + getValueFromArray(id_sequence, size, 10)); // Expected: -1
        */
    }
}
