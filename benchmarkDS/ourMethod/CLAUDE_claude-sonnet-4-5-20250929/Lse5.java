
import java.nio.charset.StandardCharsets;

public class Lse5 {
    public static String getElementAtIndex(String[] args) {
        // Predefined array of elements
        String[] elements = {"apple", "banana", "cherry", "date", "elderberry"};
        
        // Validate that we have at least one argument
        if (args == null || args.length == 0) {
            return "Error: No index provided";
        }
        
        // Validate and parse the index
        int index;
        try {
            // Parse the first argument as integer
            index = Integer.parseInt(args[0]);
        } catch (NumberFormatException e) {
            return "Error: Invalid index format";
        }
        
        // Bounds checking - ensure index is within valid range
        if (index < 0 || index >= elements.length) {
            return "Error: Index out of bounds";
        }
        
        // Return the element at the validated index
        return elements[index];
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid index 0
        String[] test1 = {"0"};
        System.out.println("Test 1 (index 0): " + getElementAtIndex(test1));
        
        // Test case 2: Valid index 2
        String[] test2 = {"2"};
        System.out.println("Test 2 (index 2): " + getElementAtIndex(test2));
        
        // Test case 3: Valid index 4 (last element)
        String[] test3 = {"4"};
        System.out.println("Test 3 (index 4): " + getElementAtIndex(test3));
        
        // Test case 4: Invalid index (out of bounds)
        String[] test4 = {"10"};
        System.out.println("Test 4 (index 10): " + getElementAtIndex(test4));
        
        // Test case 5: Invalid input (not a number)
        String[] test5 = {"abc"};
        System.out.println("Test 5 (invalid): " + getElementAtIndex(test5));
    }
}
