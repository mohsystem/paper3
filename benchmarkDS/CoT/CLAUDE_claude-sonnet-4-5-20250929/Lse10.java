
import java.util.Scanner;

public class Lse10 {
    public static int getValueFromArray(int[] array, int size, int index) {
        // Security: Validate index to prevent out-of-bounds access
        if (array == null || index < 0 || index >= size || index >= array.length) {
            return -1; // Return error code for invalid access
        }
        return array[index];
    }
    
    public static void main(String[] args) {
        int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        int size = id_sequence.length;
        
        // Test cases
        System.out.println("Test Case 1 (index 0): " + getValueFromArray(id_sequence, size, 0));
        System.out.println("Test Case 2 (index 5): " + getValueFromArray(id_sequence, size, 5));
        System.out.println("Test Case 3 (index 9): " + getValueFromArray(id_sequence, size, 9));
        System.out.println("Test Case 4 (index -1): " + getValueFromArray(id_sequence, size, -1));
        System.out.println("Test Case 5 (index 15): " + getValueFromArray(id_sequence, size, 15));
        
        // Read from standard input
        Scanner scanner = new Scanner(System.in);
        System.out.print("Enter an index: ");
        if (scanner.hasNextInt()) {
            int index = scanner.nextInt();
            int result = getValueFromArray(id_sequence, size, index);
            scanner.close();
            System.exit(result);
        } else {
            scanner.close();
            System.exit(-1);
        }
    }
}
