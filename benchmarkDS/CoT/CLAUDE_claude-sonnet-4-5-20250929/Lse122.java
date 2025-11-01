
public class Lse122 {
    public static void memoryOperation() {
        // Allocate two chunks of memory each of size 10
        byte[] chunk1 = new byte[10];
        byte[] chunk2 = new byte[10];
        
        // Initialize first chunk with some data
        for (int i = 0; i < chunk1.length; i++) {
            chunk1[i] = (byte) i;
        }
        
        // Copy contents from chunk1 to chunk2
        System.arraycopy(chunk1, 0, chunk2, 0, chunk1.length);
        
        // In Java, memory is managed by garbage collector
        // Setting references to null helps GC
        chunk1 = null;
        chunk2 = null;
        
        // Suggest garbage collection (not guaranteed to run immediately)
        System.gc();
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Basic memory operation");
        memoryOperation();
        
        System.out.println("Test Case 2: Multiple operations");
        memoryOperation();
        
        System.out.println("Test Case 3: Sequential operations");
        memoryOperation();
        
        System.out.println("Test Case 4: Memory stress test");
        memoryOperation();
        
        System.out.println("Test Case 5: Final operation");
        memoryOperation();
        
        System.out.println("All test cases completed successfully");
    }
}
