/**
 * Lse122.java
 * This code demonstrates basic memory management simulation in Java.
 * It allocates two byte arrays, copies the content of one to the other,
 * and then makes them eligible for garbage collection.
 */
public class Lse122 {

    /**
     * Allocates two byte arrays ("chunks"), copies content from the first to the second,
     * and then dereferences them to allow garbage collection, simulating freeing memory.
     */
    public static void manageMemory() {
        final int CHUNK_SIZE = 10;
        byte[] chunk1 = null;
        byte[] chunk2 = null;

        try {
            // 1. Allocate two chunks of memory each of size 10.
            chunk1 = new byte[CHUNK_SIZE];
            chunk2 = new byte[CHUNK_SIZE];
            System.out.println("Allocated two chunks of size " + CHUNK_SIZE);

            // Initialize chunk1 with some data for demonstration
            for (int i = 0; i < CHUNK_SIZE; i++) {
                chunk1[i] = (byte) (i + 1);
            }
            System.out.println("Initialized chunk1.");

            // 2. Copy the contents of the first chunk into the second chunk.
            // System.arraycopy is a safe, bounds-checked operation.
            System.arraycopy(chunk1, 0, chunk2, 0, CHUNK_SIZE);
            System.out.println("Copied contents of chunk1 to chunk2.");

            // Verification step
            boolean verified = java.util.Arrays.equals(chunk1, chunk2);
            if (verified) {
                System.out.println("Verification successful: chunk2 is a copy of chunk1.");
            } else {
                System.out.println("Verification failed: copy was not successful.");
            }

        } catch (OutOfMemoryError e) {
            System.err.println("Error: Failed to allocate memory.");
            // No need to free, GC will handle it.
            return;
        }

        // 3. Free the first chunk.
        // In Java, "freeing" memory means making an object eligible for garbage
        // collection by removing all references to it.
        chunk1 = null;
        System.out.println("Freed chunk1 (by setting its reference to null).");

        // 4. Free the second chunk.
        chunk2 = null;
        System.out.println("Freed chunk2 (by setting its reference to null).");
        System.out.println("Memory management simulation complete.");
    }

    /**
     * Main method to run test cases.
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        for (int i = 0; i < 5; i++) {
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            manageMemory();
        }
    }
}