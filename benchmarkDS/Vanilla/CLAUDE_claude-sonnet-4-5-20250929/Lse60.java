
public class Lse60 {
    public static void processBuffer(int size) {
        byte[] buffer = new byte[size];
        
        System.out.println("Buffer contents:");
        for (int i = 0; i < buffer.length; i++) {
            System.out.print(buffer[i] + " ");
            if ((i + 1) % 20 == 0) {
                System.out.println();
            }
        }
        System.out.println();
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: Buffer size 10");
        processBuffer(10);
        
        System.out.println("\\nTest Case 2: Buffer size 25");
        processBuffer(25);
        
        System.out.println("\\nTest Case 3: Buffer size 50");
        processBuffer(50);
        
        System.out.println("\\nTest Case 4: Buffer size 5");
        processBuffer(5);
        
        System.out.println("\\nTest Case 5: Buffer size 100");
        processBuffer(100);
        
        // If command line argument is provided
        if (args.length > 0) {
            try {
                int size = Integer.parseInt(args[0]);
                System.out.println("\\nCommand line argument: Buffer size " + size);
                processBuffer(size);
            } catch (NumberFormatException e) {
                System.err.println("Invalid buffer size argument");
            }
        }
    }
}
