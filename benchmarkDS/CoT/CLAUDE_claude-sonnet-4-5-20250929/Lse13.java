
import java.util.Scanner;

public class Lse13 {
    private static final int[] id_sequence = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    
    public static Integer getValueAtIndex(int index) {
        // Input validation: check for negative index
        if (index < 0) {
            System.out.println("Error: Index cannot be negative");
            return null;
        }
        
        // Boundary check: ensure index is within array bounds
        if (index >= id_sequence.length) {
            System.out.println("Error: Index out of bounds");
            return null;
        }
        
        return id_sequence[index];
    }
    
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        
        // Test cases
        System.out.println("=== Running Test Cases ===");
        testCase(0, "Valid index 0");
        testCase(5, "Valid index 5");
        testCase(9, "Valid index at boundary");
        testCase(-1, "Invalid negative index");
        testCase(15, "Invalid index out of bounds");
        
        // Interactive mode
        System.out.println("\\n=== Interactive Mode ===");
        System.out.print("Enter an index value: ");
        
        try {
            if (scanner.hasNextInt()) {
                int index = scanner.nextInt();
                Integer result = getValueAtIndex(index);
                if (result != null) {
                    System.out.println("Value at index " + index + ": " + result);
                }
            } else {
                System.out.println("Error: Invalid input. Please enter an integer.");
            }
        } catch (Exception e) {
            System.out.println("Error: Invalid input");
        } finally {
            scanner.close();
        }
    }
    
    private static void testCase(int index, String description) {
        System.out.println("\\nTest: " + description);
        Integer result = getValueAtIndex(index);
        if (result != null) {
            System.out.println("Result: " + result);
        }
    }
}
