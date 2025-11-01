
import java.util.Arrays;

public class Lse121 {
    // Secure implementation that demonstrates memory allocation and copying
    // This Java version uses byte arrays to simulate memory chunks
    
    public static void demonstrateMemoryCopy(int size) {
        // Input validation: ensure size is reasonable and positive
        if (size <= 0 || size > 1024 * 1024) { // Limit to 1MB to prevent excessive allocation
            System.err.println("Invalid size");
            return;
        }
        
        // Allocate first chunk and initialize with sample data
        byte[] chunk1 = new byte[size];
        for (int i = 0; i < size; i++) {
            chunk1[i] = (byte) (i % 256); // Initialize with bounded values
        }
        
        // Allocate second chunk
        byte[] chunk2 = new byte[size];
        
        // Securely copy contents from chunk1 to chunk2 with bounds checking
        // System.arraycopy performs bounds checking internally
        System.arraycopy(chunk1, 0, chunk2, 0, size);
        
        // Print contents of chunk2 (limited to prevent excessive output)
        System.out.println("Contents of chunk2 after copy: " + 
            Arrays.toString(Arrays.copyOf(chunk2, Math.min(10, size))));
        
        // Clear chunk1 before nullifying (simulate freeing)
        // This is important for security when dealing with sensitive data
        Arrays.fill(chunk1, (byte) 0);
        chunk1 = null;
        
        // Print contents of chunk2 again (still valid)
        System.out.println("Contents of chunk2 after chunk1 freed: " + 
            Arrays.toString(Arrays.copyOf(chunk2, Math.min(10, size))));
        
        // Clear and nullify chunk2 (simulate freeing)
        Arrays.fill(chunk2, (byte) 0);
        chunk2 = null;
    }
    
    public static void main(String[] args) {
        // Test case 1: Small allocation
        System.out.println("Test 1: Size 10");
        demonstrateMemoryCopy(10);
        
        // Test case 2: Medium allocation
        System.out.println("\\nTest 2: Size 100");
        demonstrateMemoryCopy(100);
        
        // Test case 3: Larger allocation
        System.out.println("\\nTest 3: Size 1000");
        demonstrateMemoryCopy(1000);
        
        // Test case 4: Edge case - size 1
        System.out.println("\\nTest 4: Size 1");
        demonstrateMemoryCopy(1);
        
        // Test case 5: Invalid case - negative size
        System.out.println("\\nTest 5: Size -1 (invalid)");
        demonstrateMemoryCopy(-1);
    }
}
