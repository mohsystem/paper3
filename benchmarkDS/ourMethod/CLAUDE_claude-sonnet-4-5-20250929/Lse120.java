
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse120 {
    // Allocate two chunks of memory, copy first to second, and print second
    public static String allocateAndCopy(String sourceData) {
        if (sourceData == null) {
            return "";
        }
        
        // Step 1: Allocate first chunk and initialize with source data
        // Using byte array for memory representation
        byte[] firstChunk = sourceData.getBytes(StandardCharsets.UTF_8);
        
        // Step 2: Allocate second chunk with same size as first
        // Size validation to prevent excessive allocation
        if (firstChunk.length > 1048576) { // 1MB limit
            throw new IllegalArgumentException("Data size exceeds maximum allowed");
        }
        byte[] secondChunk = new byte[firstChunk.length];
        
        // Step 3: Copy contents from first chunk to second chunk
        // Using System.arraycopy for bounds-safe copying
        System.arraycopy(firstChunk, 0, secondChunk, 0, firstChunk.length);
        
        // Step 4: Convert second chunk back to string for printing
        String result = new String(secondChunk, StandardCharsets.UTF_8);
        
        // Clear sensitive data from memory
        Arrays.fill(firstChunk, (byte) 0);
        Arrays.fill(secondChunk, (byte) 0);
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal string
        System.out.println("Test 1: " + allocateAndCopy("Hello World"));
        
        // Test case 2: Empty string
        System.out.println("Test 2: " + allocateAndCopy(""));
        
        // Test case 3: String with special characters
        System.out.println("Test 3: " + allocateAndCopy("Test@123!#$"));
        
        // Test case 4: Null input
        System.out.println("Test 4: " + allocateAndCopy(null));
        
        // Test case 5: Unicode string
        System.out.println("Test 5: " + allocateAndCopy("Hello 世界 🌍"));
    }
}
