
import java.util.Scanner;

public class Task159 {
    private static final int BUFFER_SIZE = 100;
    private int[] buffer;
    
    public Task159() {
        buffer = new int[BUFFER_SIZE];
        // Initialize buffer with sample data
        for (int i = 0; i < BUFFER_SIZE; i++) {
            buffer[i] = i * 10;
        }
    }
    
    public Integer readFromBuffer(int index) {
        // Secure bounds checking
        if (index < 0 || index >= buffer.length) {
            System.out.println("Error: Index out of bounds. Valid range: 0-" + (buffer.length - 1));
            return null;
        }
        return buffer[index];
    }
    
    public int getBufferSize() {
        return buffer.length;
    }
    
    public static void main(String[] args) {
        Task159 task = new Task159();
        
        System.out.println("=== Test Cases ===\\n");
        
        // Test case 1: Valid index at start
        System.out.println("Test 1 - Valid index (0):");
        Integer result1 = task.readFromBuffer(0);
        if (result1 != null) {
            System.out.println("Value at index 0: " + result1);
        }
        System.out.println();
        
        // Test case 2: Valid index in middle
        System.out.println("Test 2 - Valid index (50):");
        Integer result2 = task.readFromBuffer(50);
        if (result2 != null) {
            System.out.println("Value at index 50: " + result2);
        }
        System.out.println();
        
        // Test case 3: Valid index at end
        System.out.println("Test 3 - Valid index (99):");
        Integer result3 = task.readFromBuffer(99);
        if (result3 != null) {
            System.out.println("Value at index 99: " + result3);
        }
        System.out.println();
        
        // Test case 4: Invalid negative index
        System.out.println("Test 4 - Invalid negative index (-5):");
        Integer result4 = task.readFromBuffer(-5);
        System.out.println();
        
        // Test case 5: Invalid index beyond bounds
        System.out.println("Test 5 - Invalid index beyond bounds (150):");
        Integer result5 = task.readFromBuffer(150);
        System.out.println();
        
        // Interactive mode (commented out for automated testing)
        /*
        Scanner scanner = new Scanner(System.in);
        System.out.println("Buffer size: " + task.getBufferSize());
        
        while (true) {
            System.out.print("Enter index to read (or -1 to exit): ");
            int index = scanner.nextInt();
            
            if (index == -1) {
                break;
            }
            
            Integer value = task.readFromBuffer(index);
            if (value != null) {
                System.out.println("Value at index " + index + ": " + value);
            }
        }
        scanner.close();
        */
    }
}
