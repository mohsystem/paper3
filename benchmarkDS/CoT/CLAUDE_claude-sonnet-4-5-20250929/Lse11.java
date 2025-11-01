
import java.util.Scanner;

public class Lse11 {
    public static int getValueFromArray(int[] array, int size, int index) {
        // Validate parameters to prevent security vulnerabilities
        if (array == null) {
            return -1;
        }
        
        // Validate index is within valid bounds to prevent buffer overflow
        if (index < 0 || index >= size || index >= array.length) {
            return -1;
        }
        
        return array[index];
    }
    
    public static void main(String[] args) {
        // Define the id_sequence array
        int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        int size = id_sequence.length;
        
        // Test cases
        System.out.println("Test Case 1 - Index 0: " + getValueFromArray(id_sequence, size, 0));
        System.out.println("Test Case 2 - Index 5: " + getValueFromArray(id_sequence, size, 5));
        System.out.println("Test Case 3 - Index 9: " + getValueFromArray(id_sequence, size, 9));
        System.out.println("Test Case 4 - Index -1 (invalid): " + getValueFromArray(id_sequence, size, -1));
        System.out.println("Test Case 5 - Index 15 (out of bounds): " + getValueFromArray(id_sequence, size, 15));
        
        // Read input from user
        Scanner scanner = new Scanner(System.in);
        System.out.print("Enter an index: ");
        
        try {
            int index = scanner.nextInt();
            int result = getValueFromArray(id_sequence, size, index);
            scanner.close();
            System.exit(result);
        } catch (Exception e) {
            scanner.close();
            System.exit(-1);
        }
    }
}
