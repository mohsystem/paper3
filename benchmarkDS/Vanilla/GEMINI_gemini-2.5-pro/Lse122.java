public class Lse122 {

    /**
     * Allocates two byte arrays, copies content from the first to the second,
     * and then makes them eligible for garbage collection.
     */
    public static void demonstrateMemoryOperations() {
        // 1. Allocate two chunks of memory each of size 10.
        // In Java, this is typically done using arrays. The JVM's garbage collector manages memory.
        byte[] chunk1 = new byte[10];
        byte[] chunk2 = new byte[10];
        System.out.println("Allocated two byte arrays (chunks) of size 10.");

        // For demonstration, fill the first chunk with some data.
        for (int i = 0; i < chunk1.length; i++) {
            chunk1[i] = (byte)(i + 1);
        }

        // 2. Copy the contents of the first chunk into the second chunk.
        System.arraycopy(chunk1, 0, chunk2, 0, 10);
        System.out.println("Copied contents from chunk1 to chunk2.");

        // 3. Free the first chunk.
        // In Java, "freeing" memory means making an object eligible for garbage collection
        // by removing all references to it. Setting the reference to null is the common way.
        chunk1 = null;
        System.out.println("Freed the first chunk (set reference to null).");

        // 4. Free the second chunk.
        chunk2 = null;
        System.out.println("Freed the second chunk (set reference to null).");
        
        // The garbage collector will reclaim the memory automatically at a later time.
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Test Cases ---");
        for (int i = 1; i <= 5; i++) {
            System.out.println("\n--- Test Case " + i + " ---");
            demonstrateMemoryOperations();
            System.out.println("--- End of Test Case " + i + " ---");
        }
    }
}