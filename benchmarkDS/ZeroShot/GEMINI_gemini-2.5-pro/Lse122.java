class Lse122 {
    /**
     * Demonstrates secure memory allocation, copy, and release in Java.
     * In Java, memory is managed by the Garbage Collector (GC). "Allocating"
     * means creating an object. "Freeing" means making an object eligible
     * for garbage collection by removing all references to it.
     */
    public static void secureMemoryOperations() {
        System.out.println("--- Java: Starting memory operations ---");

        // Use a try-finally block to ensure resources are handled even if errors occur.
        byte[] chunk1 = null;
        byte[] chunk2 = null;
        try {
            // 1. Allocate two chunks of memory each of size 10.
            chunk1 = new byte[10];
            System.out.println("Step 1: Allocated chunk1 (byte array of size 10).");
            
            chunk2 = new byte[10];
            System.out.println("Step 2: Allocated chunk2 (byte array of size 10).");

            // Initialize chunk1 with some data for demonstration
            for (int i = 0; i < chunk1.length; i++) {
                chunk1[i] = (byte)(i + 'A');
            }
            System.out.println("Initialized chunk1.");

            // 2. Copy the contents of the first chunk into the second chunk.
            // System.arraycopy is a safe, built-in function that performs bounds checking.
            System.arraycopy(chunk1, 0, chunk2, 0, 10);
            System.out.println("Step 3: Copied contents from chunk1 to chunk2.");

        } catch (OutOfMemoryError e) {
            System.err.println("Error: Failed to allocate memory. " + e.getMessage());
        } catch (Exception e) {
            System.err.println("An unexpected error occurred: " + e.getMessage());
        } finally {
            // 3. Free the first chunk.
            // In Java, this means making it eligible for garbage collection.
            chunk1 = null;
            System.out.println("Step 4: Freed chunk1 by setting its reference to null.");

            // 4. Free the second chunk.
            chunk2 = null;
            System.out.println("Step 5: Freed chunk2 by setting its reference to null.");
        }
        
        System.out.println("--- Java: Finished memory operations ---");
    }

    public static void main(String[] args) {
        for (int i = 0; i < 5; i++) {
            System.out.println("\nTest Case " + (i + 1));
            secureMemoryOperations();
        }
    }
}