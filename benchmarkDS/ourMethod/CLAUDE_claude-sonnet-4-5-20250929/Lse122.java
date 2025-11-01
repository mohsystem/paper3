
import java.nio.ByteBuffer;
import java.util.Arrays;

public class Lse122 {
    // Function to allocate, copy, and free memory chunks
    public static void allocateCopyAndFree() {
        // Allocate two chunks of memory, each of size 10 bytes
        // Using ByteBuffer provides bounds-checked memory operations
        ByteBuffer chunk1 = null;
        ByteBuffer chunk2 = null;
        
        try {
            // Allocate first chunk with bounds checking
            chunk1 = ByteBuffer.allocate(10);
            if (chunk1 == null) {
                throw new OutOfMemoryError("Failed to allocate chunk1");
            }
            
            // Initialize first chunk with sample data for testing
            for (int i = 0; i < 10; i++) {
                chunk1.put(i, (byte) i);
            }
            
            // Allocate second chunk with bounds checking
            chunk2 = ByteBuffer.allocate(10);
            if (chunk2 == null) {
                throw new OutOfMemoryError("Failed to allocate chunk2");
            }
            
            // Copy contents from chunk1 to chunk2 with bounds validation
            // ByteBuffer ensures no buffer overflow during copy
            byte[] tempBuffer = new byte[10];
            chunk1.position(0);
            chunk1.get(tempBuffer, 0, 10); // Bounds-checked read
            chunk2.position(0);
            chunk2.put(tempBuffer, 0, 10); // Bounds-checked write
            
            // Clear sensitive data from chunk1 before allowing GC
            Arrays.fill(chunk1.array(), (byte) 0);
            chunk1 = null; // Release reference for garbage collection
            
            // Clear sensitive data from chunk2 before allowing GC
            Arrays.fill(chunk2.array(), (byte) 0);
            chunk2 = null; // Release reference for garbage collection
            
        } catch (OutOfMemoryError e) {
            System.err.println("Memory allocation failed");
            // Ensure cleanup on error
            if (chunk1 != null) {
                Arrays.fill(chunk1.array(), (byte) 0);
            }
            if (chunk2 != null) {
                Arrays.fill(chunk2.array(), (byte) 0);
            }
        } catch (Exception e) {
            System.err.println("Operation failed");
            // Ensure cleanup on error
            if (chunk1 != null) {
                Arrays.fill(chunk1.array(), (byte) 0);
            }
            if (chunk2 != null) {
                Arrays.fill(chunk2.array(), (byte) 0);
            }
        } finally {
            // Suggest garbage collection (JVM will decide when to actually collect)
            System.gc();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal operation
        System.out.println("Test 1: Normal allocation, copy, and free");
        allocateCopyAndFree();
        
        // Test case 2: Repeated operations
        System.out.println("Test 2: Repeated operations");
        allocateCopyAndFree();
        
        // Test case 3: Multiple sequential calls
        System.out.println("Test 3: Multiple sequential calls");
        for (int i = 0; i < 3; i++) {
            allocateCopyAndFree();
        }
        
        // Test case 4: Single operation with delay
        System.out.println("Test 4: Single operation");
        allocateCopyAndFree();
        
        // Test case 5: Final operation
        System.out.println("Test 5: Final operation");
        allocateCopyAndFree();
        
        System.out.println("All tests completed successfully");
    }
}
