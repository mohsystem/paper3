
public class Lse61 {
    public static void allocateAndFreeBuffer(int bufferSize) {
        if (bufferSize <= 0) {
            System.out.println("Invalid buffer size");
            return;
        }
        
        // Allocate buffer
        byte[] buffer = new byte[bufferSize];
        System.out.println("Buffer of size " + bufferSize + " allocated");
        
        // Use the buffer (example)
        for (int i = 0; i < Math.min(bufferSize, 10); i++) {
            buffer[i] = (byte) i;
        }
        
        // In Java, memory is managed by garbage collector
        // Setting to null allows garbage collection
        buffer = null;
        System.out.println("Buffer freed (eligible for garbage collection)");
        
        // Note: Freeing in a loop would cause issues in manual memory management languages
        // Java handles this through garbage collection
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Small buffer");
        allocateAndFreeBuffer(10);
        
        System.out.println("\\nTest Case 2: Medium buffer");
        allocateAndFreeBuffer(100);
        
        System.out.println("\\nTest Case 3: Large buffer");
        allocateAndFreeBuffer(1000);
        
        System.out.println("\\nTest Case 4: Invalid buffer (zero)");
        allocateAndFreeBuffer(0);
        
        System.out.println("\\nTest Case 5: Invalid buffer (negative)");
        allocateAndFreeBuffer(-10);
    }
}
