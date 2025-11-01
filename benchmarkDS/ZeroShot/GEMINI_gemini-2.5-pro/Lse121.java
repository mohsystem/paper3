import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse121 {

    /**
     * Demonstrates memory allocation, copying, and freeing concepts in Java.
     * In Java, memory management is handled by the Garbage Collector (GC).
     * "Freeing" memory is done by removing all references to an object,
     * making it eligible for garbage collection.
     *
     * @param content The string content to use for the memory chunks.
     */
    public static void demonstrateMemoryOperations(String content) {
        System.out.println("---- Demonstrating for content: \"" + content + "\" ----");
        
        // Use byte arrays to represent chunks of memory.
        // Convert the string to bytes using a standard charset.
        byte[] chunk1 = content.getBytes(StandardCharsets.UTF_8);
        
        // 1. Allocate a second chunk of memory of the same size.
        // Handle the case of an empty initial chunk.
        if (chunk1 == null) {
            System.out.println("Initial content is null. Cannot proceed.");
            return;
        }
        byte[] chunk2 = new byte[chunk1.length];

        // 2. Copy the contents of the first chunk into the second chunk.
        // System.arraycopy is a safe and efficient way to copy arrays,
        // preventing buffer overflows by checking bounds.
        System.arraycopy(chunk1, 0, chunk2, 0, chunk1.length);
        
        // 3. Print the contents of the second chunk.
        System.out.println("Contents of chunk2 before freeing chunk1: " + new String(chunk2, StandardCharsets.UTF_8));
        
        // 4. "Free" the memory allocated to the first chunk.
        // In Java, this is done by making the object eligible for garbage collection
        // by removing all references to it. Here, we set the reference to null.
        chunk1 = null;
        // At this point, the memory for the original chunk1 is not yet reclaimed,
        // but it is scheduled for collection. This does not affect chunk2.
        
        System.out.println("chunk1 has been set to null (marked for garbage collection).");

        // 5. Print the contents of the second chunk again.
        // The contents of chunk2 are independent and remain unchanged.
        System.out.println("Contents of chunk2 after freeing chunk1:  " + new String(chunk2, StandardCharsets.UTF_8));
        
        // 6. "Free" the memory allocated to the second chunk.
        chunk2 = null;
        System.out.println("chunk2 has been set to null (marked for garbage collection).");
        System.out.println("------------------------------------------\n");
    }

    public static void main(String[] args) {
        // 5 test cases
        demonstrateMemoryOperations("Hello, Secure World!");
        demonstrateMemoryOperations(""); // Empty string
        demonstrateMemoryOperations("A\0B\0C"); // String with embedded nulls
        demonstrateMemoryOperations("This is a much longer string to test a slightly larger allocation of memory.");
        demonstrateMemoryOperations("!@#$%^&*()_+-=[]{}|;':,./<>?");
    }
}