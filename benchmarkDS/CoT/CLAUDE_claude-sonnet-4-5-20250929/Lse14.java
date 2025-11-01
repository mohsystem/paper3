
import java.util.Scanner;

public class Lse14 {
    private static final int[] id_sequence = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    
    public static Integer getValueAtIndex(int index) {
        // Validate index bounds
        if (index < 0 || index >= id_sequence.length) {
            System.out.println("Error: Index out of bounds. Valid range: 0-" + (id_sequence.length - 1));
            return null;
        }
        return id_sequence[index];
    }
    
    public static Integer getUserInputAndGetValue() {
        Scanner scanner = new Scanner(System.in);
        System.out.print("Enter an index value (0-" + (id_sequence.length - 1) + "): ");
        
        try {
            int index = scanner.nextInt();
            return getValueAtIndex(index);
        } catch (Exception e) {
            System.out.println("Error: Invalid input. Please enter a valid integer.");
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid index at beginning
        System.out.println("Test 1 - Index 0: " + getValueAtIndex(0));
        
        // Test case 2: Valid index in middle
        System.out.println("Test 2 - Index 5: " + getValueAtIndex(5));
        
        // Test case 3: Valid index at end
        System.out.println("Test 3 - Index 9: " + getValueAtIndex(9));
        
        // Test case 4: Invalid negative index
        System.out.println("Test 4 - Index -1: " + getValueAtIndex(-1));
        
        // Test case 5: Invalid index beyond bounds
        System.out.println("Test 5 - Index 15: " + getValueAtIndex(15));
    }
}
