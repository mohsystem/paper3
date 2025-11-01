
public class Lse121 {
    public static void demonstrateMemoryCopy() {
        // Allocate two chunks of memory (using byte arrays in Java)
        byte[] firstChunk = new byte[100];
        byte[] secondChunk = new byte[100];
        
        // Fill first chunk with some data
        String data = "Hello, this is test data for memory operation!";
        byte[] dataBytes = data.getBytes();
        System.arraycopy(dataBytes, 0, firstChunk, 0, Math.min(dataBytes.length, firstChunk.length));
        
        // Copy contents of first chunk into second chunk
        System.arraycopy(firstChunk, 0, secondChunk, 0, firstChunk.length);
        
        // Print contents of second chunk
        System.out.println("Contents of second chunk after copy:");
        System.out.println(new String(secondChunk).trim());
        
        // Free memory of first chunk (set to null for garbage collection)
        firstChunk = null;
        System.gc(); // Suggest garbage collection
        
        // Print contents of second chunk again
        System.out.println("\\nContents of second chunk after freeing first chunk:");
        System.out.println(new String(secondChunk).trim());
        
        // Free memory of second chunk
        secondChunk = null;
        System.gc(); // Suggest garbage collection
        
        System.out.println("\\nMemory operations completed successfully.");
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        demonstrateMemoryCopy();
        
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 2:");
        demonstrateMemoryCopy();
        
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 3:");
        demonstrateMemoryCopy();
        
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 4:");
        demonstrateMemoryCopy();
        
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 5:");
        demonstrateMemoryCopy();
    }
}
