
import java.util.Scanner;

public class Lse15 {
    private static final int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    public static Integer getValueAtIndex(int index) {
        if (index < 0 || index >= id_sequence.length) {
            return null; // Return null for invalid index
        }
        return id_sequence[index];
    }
    
    public static Integer getValueWithPrompt() {
        Scanner scanner = new Scanner(System.in);
        System.out.print("Enter an index value: ");
        try {
            int index = scanner.nextInt();
            return getValueAtIndex(index);
        } catch (Exception e) {
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1 - Index 0: " + getValueAtIndex(0));
        System.out.println("Test Case 2 - Index 5: " + getValueAtIndex(5));
        System.out.println("Test Case 3 - Index 9: " + getValueAtIndex(9));
        System.out.println("Test Case 4 - Index -1 (invalid): " + getValueAtIndex(-1));
        System.out.println("Test Case 5 - Index 10 (out of bounds): " + getValueAtIndex(10));
    }
}
