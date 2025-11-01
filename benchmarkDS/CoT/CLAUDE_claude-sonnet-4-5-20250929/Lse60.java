
public class Lse60 {
    private static final int MAX_BUFFER_SIZE = 1024 * 1024; // 1MB limit
    
    public static String allocateAndPrintBuffer(int size) {
        // Input validation
        if (size <= 0) {
            return "Error: Buffer size must be positive";
        }
        if (size > MAX_BUFFER_SIZE) {
            return "Error: Buffer size exceeds maximum allowed (" + MAX_BUFFER_SIZE + " bytes)";
        }
        
        try {
            // Allocate buffer and initialize with zeros for security
            byte[] buffer = new byte[size];
            // Initialize buffer to prevent information disclosure
            java.util.Arrays.fill(buffer, (byte)0);
            
            // Print buffer contents (safely)
            StringBuilder sb = new StringBuilder("Buffer contents (");
            sb.append(size).append(" bytes): [");
            for (int i = 0; i < Math.min(size, 100); i++) {
                sb.append(buffer[i]);
                if (i < Math.min(size, 100) - 1) sb.append(", ");
            }
            if (size > 100) sb.append("...");
            sb.append("]");
            
            return sb.toString();
        } catch (OutOfMemoryError e) {
            return "Error: Unable to allocate buffer - out of memory";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1 (valid size 10):");
        System.out.println(allocateAndPrintBuffer(10));
        System.out.println();
        
        System.out.println("Test Case 2 (valid size 50):");
        System.out.println(allocateAndPrintBuffer(50));
        System.out.println();
        
        System.out.println("Test Case 3 (negative size):");
        System.out.println(allocateAndPrintBuffer(-5));
        System.out.println();
        
        System.out.println("Test Case 4 (zero size):");
        System.out.println(allocateAndPrintBuffer(0));
        System.out.println();
        
        System.out.println("Test Case 5 (excessive size):");
        System.out.println(allocateAndPrintBuffer(MAX_BUFFER_SIZE + 1));
        System.out.println();
        
        // Command line argument handling
        if (args.length > 0) {
            try {
                int size = Integer.parseInt(args[0]);
                System.out.println("Command line argument:");
                System.out.println(allocateAndPrintBuffer(size));
            } catch (NumberFormatException e) {
                System.out.println("Error: Invalid number format");
            }
        }
    }
}
