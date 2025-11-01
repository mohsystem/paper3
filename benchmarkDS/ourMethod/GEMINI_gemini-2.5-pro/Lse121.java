public class Lse121 {

    /**
     * Allocates two byte arrays, copies content from the first to the second,
     * prints the second, "frees" the first, prints the second again,
     * and finally "frees" the second.
     *
     * @param initialData The string to initialize the first memory chunk with.
     */
    public static void demonstrateMemoryOperations(String initialData) {
        System.out.println("--- Demonstrating with data: \"" + initialData + "\" ---");

        // The data to be stored in the first chunk of memory.
        byte[] dataBytes = initialData.getBytes(java.nio.charset.StandardCharsets.UTF_8);
        int size = dataBytes.length;

        // 1. Allocate two chunks of memory.
        // In Java, this is done by creating byte arrays.
        byte[] firstChunk = new byte[size];
        byte[] secondChunk = new byte[size];

        // Initialize the first chunk.
        System.arraycopy(dataBytes, 0, firstChunk, 0, size);

        // 2. Copy the contents of the first chunk into the second chunk.
        // System.arraycopy is a safe way to copy arrays, preventing buffer overflows.
        System.arraycopy(firstChunk, 0, secondChunk, 0, size);

        // 3. Print the contents of the second chunk.
        System.out.println("Content of second chunk after copy: " + new String(secondChunk, java.nio.charset.StandardCharsets.UTF_8));

        // 4. Free the memory allocated to the first chunk.
        // In Java, we can't explicitly free memory. We make it eligible for
        // garbage collection by removing references to it.
        firstChunk = null;
        System.out.println("First chunk freed (eligible for GC).");

        // 5. Print the contents of the second chunk again.
        // The content remains because the second chunk is a separate memory area.
        System.out.println("Content of second chunk after freeing first: " + new String(secondChunk, java.nio.charset.StandardCharsets.UTF_8));

        // 6. Free the memory allocated to the second chunk.
        secondChunk = null;
        System.out.println("Second chunk freed (eligible for GC).");
        System.out.println("-------------------------------------------------");
    }

    public static void main(String[] args) {
        // 5 Test Cases
        demonstrateMemoryOperations("Test case 1: Hello, Java!");
        demonstrateMemoryOperations("Test case 2: A different string.");
        demonstrateMemoryOperations("Test case 3: Short");
        demonstrateMemoryOperations("Test case 4: A much longer string to test allocation with different sizes.");
        demonstrateMemoryOperations("Test case 5: Final test case with symbols!@#$%");
    }
}