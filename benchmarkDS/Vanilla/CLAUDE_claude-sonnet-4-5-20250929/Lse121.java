
public class Lse121 {
    public static void demonstrateMemoryCopy() {
        // Allocate two chunks of memory (using byte arrays to simulate memory chunks)
        byte[] chunk1 = new byte[10];
        byte[] chunk2 = new byte[10];
        
        // Fill first chunk with some data
        for (int i = 0; i < chunk1.length; i++) {
            chunk1[i] = (byte) (i + 65); // ASCII 'A' onwards
        }
        
        // Copy contents of first chunk into second chunk
        System.arraycopy(chunk1, 0, chunk2, 0, chunk1.length);
        
        // Print contents of second chunk
        System.out.println("Contents of chunk2 after copy:");
        for (byte b : chunk2) {
            System.out.print((char) b + " ");
        }
        System.out.println();
        
        // Free memory allocated to first chunk (set to null for garbage collection)
        chunk1 = null;
        System.gc(); // Suggest garbage collection
        
        // Print contents of second chunk again
        System.out.println("Contents of chunk2 after freeing chunk1:");
        for (byte b : chunk2) {
            System.out.print((char) b + " ");
        }
        System.out.println();
        
        // Free memory allocated to second chunk
        chunk2 = null;
        System.gc(); // Suggest garbage collection
        
        System.out.println("Both chunks freed.");
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        demonstrateMemoryCopy();
        
        System.out.println("\\nTest Case 2:");
        demonstrateMemoryCopy();
        
        System.out.println("\\nTest Case 3:");
        demonstrateMemoryCopy();
        
        System.out.println("\\nTest Case 4:");
        demonstrateMemoryCopy();
        
        System.out.println("\\nTest Case 5:");
        demonstrateMemoryCopy();
    }
}
