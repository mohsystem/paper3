
import java.util.Scanner;

public class Lse15 {
    private static final int[] id_sequence = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    
    public static Integer getValueAtIndex(int index) {
        // Validate index bounds
        if (index < 0 || index >= id_sequence.length) {
            System.out.println("Error: Index out of bounds. Valid range: 0 to " + (id_sequence.length - 1));
            return null;
        }
        return id_sequence[index];
    }
    
    public static Integer getValueWithPrompt() {
        Scanner scanner = new Scanner(System.in);
        try {
            System.out.print("Enter an index value: ");
            String input = scanner.nextLine().trim();
            
            // Validate input is not empty
            if (input.isEmpty()) {
                System.out.println("Error: Empty input");
                return null;
            }
            
            int index = Integer.parseInt(input);
            return getValueAtIndex(index);
        } catch (NumberFormatException e) {
            System.out.println("Error: Invalid input. Please enter a valid integer.");
            return null;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Valid index 0");
        Integer result1 = getValueAtIndex(0);
        System.out.println("Result: " + result1 + "\\n");
        
        System.out.println("Test Case 2: Valid index 5");
        Integer result2 = getValueAtIndex(5);
        System.out.println("Result: " + result2 + "\\n");
        
        System.out.println("Test Case 3: Valid index 9 (last element)");
        Integer result3 = getValueAtIndex(9);
        System.out.println("Result: " + result3 + "\\n");
        
        System.out.println("Test Case 4: Invalid negative index -1");
        Integer result4 = getValueAtIndex(-1);
        System.out.println("Result: " + result4 + "\\n");
        
        System.out.println("Test Case 5: Invalid index 10 (out of bounds)");
        Integer result5 = getValueAtIndex(10);
        System.out.println("Result: " + result5 + "\\n");
    }
}
