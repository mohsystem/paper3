
import java.util.Scanner;

public class Lse11 {
    // Function to get value from array with bounds checking
    // Returns the value at the given index if valid, otherwise returns -1
    private static int getValueFromArray(int[] array, int size, int index) {
        // Security: Validate array is not null
        if (array == null) {
            System.err.println("Error: Array is null");
            return -1;
        }
        
        // Security: Validate size parameter matches actual array length
        if (size != array.length) {
            System.err.println("Error: Size mismatch");
            return -1;
        }
        
        // Security: Bounds checking to prevent out-of-bounds access
        if (index < 0 || index >= size) {
            System.err.println("Error: Index out of bounds");
            return -1;
        }
        
        // Safe to access array element after validation
        return array[index];
    }
    
    public static void main(String[] args) {
        // Predefined array for testing
        int[] id_sequence = {10, 20, 30, 40, 50};
        
        // Security: Use try-with-resources to ensure Scanner is closed
        try (Scanner scanner = new Scanner(System.in)) {
            System.out.print("Enter index: ");
            
            // Security: Validate input is an integer
            if (!scanner.hasNextInt()) {
                System.err.println("Error: Invalid input");
                System.exit(1);
            }
            
            int index = scanner.nextInt();
            
            // Call function and get result
            int result = getValueFromArray(id_sequence, id_sequence.length, index);
            
            // Exit with the result as exit code
            // Note: Exit codes are typically 0-255, negative values may wrap
            System.exit(result >= 0 ? result : 1);
            
        } catch (Exception e) {
            // Security: Catch any unexpected exceptions
            System.err.println("Error occurred");
            System.exit(1);
        }
        
        // Test cases (comment out the interactive part above to run these)
        /*
        System.out.println("Test 1 - Valid index 0: " + getValueFromArray(id_sequence, id_sequence.length, 0)); // Expected: 10
        System.out.println("Test 2 - Valid index 4: " + getValueFromArray(id_sequence, id_sequence.length, 4)); // Expected: 50
        System.out.println("Test 3 - Invalid index -1: " + getValueFromArray(id_sequence, id_sequence.length, -1)); // Expected: -1
        System.out.println("Test 4 - Invalid index 5: " + getValueFromArray(id_sequence, id_sequence.length, 5)); // Expected: -1
        System.out.println("Test 5 - Null array: " + getValueFromArray(null, 0, 0)); // Expected: -1
        */
    }
}
