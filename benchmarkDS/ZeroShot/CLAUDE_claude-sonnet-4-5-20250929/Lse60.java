
public class Lse60 {
    public static void printBuffer(int size) {
        if (size <= 0) {
            System.out.println("Buffer size must be positive");
            return;
        }
        
        // Allocate buffer with specified size
        byte[] buffer = new byte[size];
        
        // Initialize buffer with zeros (default in Java)
        // Print buffer contents
        System.out.print("Buffer contents: [");
        for (int i = 0; i < buffer.length; i++) {
            System.out.print(buffer[i]);
            if (i < buffer.length - 1) {
                System.out.print(", ");
            }
        }
        System.out.println("]");
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        printBuffer(5);
        
        System.out.println("\\nTest Case 2:");
        printBuffer(10);
        
        System.out.println("\\nTest Case 3:");
        printBuffer(1);
        
        System.out.println("\\nTest Case 4:");
        printBuffer(0);
        
        System.out.println("\\nTest Case 5:");
        printBuffer(20);
        
        // Command line argument handling
        if (args.length > 0) {
            try {
                int size = Integer.parseInt(args[0]);
                System.out.println("\\nCommand line argument:");
                printBuffer(size);
            } catch (NumberFormatException e) {
                System.out.println("Invalid argument. Please provide a valid integer.");
            }
        }
    }
}
