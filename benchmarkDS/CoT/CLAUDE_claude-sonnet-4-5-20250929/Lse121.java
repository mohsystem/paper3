
public class Lse121 {
    public static void demonstrateMemoryOperations() {
        try {
            // Allocate first chunk with initial data
            String firstChunk = "Hello, Secure World!";
            System.out.println("First chunk allocated: " + firstChunk);
            
            // Allocate second chunk and copy contents
            String secondChunk = new String(firstChunk);
            System.out.println("Second chunk after copy: " + secondChunk);
            
            // Simulate freeing first chunk (set to null)
            firstChunk = null;
            System.gc(); // Suggest garbage collection
            System.out.println("First chunk freed");
            
            // Print second chunk after first is freed
            System.out.println("Second chunk after first freed: " + secondChunk);
            
            // Free second chunk
            secondChunk = null;
            System.gc();
            System.out.println("Second chunk freed");
            
        } catch (Exception e) {
            System.err.println("Error in memory operations: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Basic operation ===");
        demonstrateMemoryOperations();
        
        System.out.println("\\n=== Test Case 2: Basic operation ===");
        demonstrateMemoryOperations();
        
        System.out.println("\\n=== Test Case 3: Basic operation ===");
        demonstrateMemoryOperations();
        
        System.out.println("\\n=== Test Case 4: Basic operation ===");
        demonstrateMemoryOperations();
        
        System.out.println("\\n=== Test Case 5: Basic operation ===");
        demonstrateMemoryOperations();
    }
}
