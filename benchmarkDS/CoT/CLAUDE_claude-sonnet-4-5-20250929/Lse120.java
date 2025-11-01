
public class Lse120 {
    public static void allocateAndCopy(String data) {
        if (data == null) {
            System.out.println("Error: Input data is null");
            return;
        }
        
        // Limit size to prevent excessive memory allocation
        final int MAX_SIZE = 1024;
        if (data.length() > MAX_SIZE) {
            System.out.println("Error: Input data exceeds maximum size");
            return;
        }
        
        try {
            // Allocate first chunk
            byte[] chunk1 = data.getBytes("UTF-8");
            
            // Allocate second chunk with same size
            byte[] chunk2 = new byte[chunk1.length];
            
            // Safely copy contents
            System.arraycopy(chunk1, 0, chunk2, 0, chunk1.length);
            
            // Print contents of second chunk
            System.out.println("Contents of second chunk: " + new String(chunk2, "UTF-8"));
            
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal string
        System.out.println("Test 1:");
        allocateAndCopy("Hello World");
        
        // Test case 2: Empty string
        System.out.println("\\nTest 2:");
        allocateAndCopy("");
        
        // Test case 3: String with special characters
        System.out.println("\\nTest 3:");
        allocateAndCopy("Test@123!#$");
        
        // Test case 4: Null input
        System.out.println("\\nTest 4:");
        allocateAndCopy(null);
        
        // Test case 5: Unicode string
        System.out.println("\\nTest 5:");
        allocateAndCopy("Hello 世界");
    }
}
