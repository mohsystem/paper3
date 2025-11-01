
import java.util.Scanner;

public class Lse12 {
    // Returns the value at the given index from the array
    // Security: Validates index bounds to prevent out-of-bounds access
    public static int getValueFromArray(int[] array, int size, int index) {
        // Validate input parameters to prevent buffer overflow/underflow
        if (array == null) {
            System.err.println("Error: Null array provided");
            return -1;
        }
        
        // Ensure size parameter matches actual array length
        if (size != array.length) {
            System.err.println("Error: Size mismatch");
            return -1;
        }
        
        // Bounds checking: ensure index is within valid range [0, size)
        if (index < 0 || index >= size) {
            System.err.println("Error: Index out of bounds");
            return -1;
        }
        
        return array[index];
    }
    
    public static void main(String[] args) {
        // Predefined test array for security - avoid dynamic sizing from untrusted input
        int[] testArray = {10, 20, 30, 40, 50};
        int size = testArray.length;
        
        Scanner scanner = null;
        try {
            scanner = new Scanner(System.in);
            
            // Validate input exists
            if (!scanner.hasNextInt()) {
                System.err.println("Error: Invalid input");
                System.exit(1);
            }
            
            // Read index from standard input - treat as untrusted
            int index = scanner.nextInt();
            
            // Call function with bounds-checked parameters
            int result = getValueFromArray(testArray, size, index);
            
            // Return result as exit code (constrained to valid range)
            // Exit codes are typically 0-255, so we use modulo to ensure valid range
            System.exit(Math.abs(result) % 256);
            
        } catch (Exception e) {
            // Catch all exceptions, log generic error (no internal details leaked)
            System.err.println("Error: Processing failed");
            System.exit(1);
        } finally {
            // Ensure scanner is properly closed
            if (scanner != null) {
                scanner.close();
            }
        }
    }
}
