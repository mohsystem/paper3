public class Lse121 {

    public static void demonstrateMemoryManagement(String initialData) {
        System.out.println("Processing data: \"" + initialData + "\"");

        // 1. Allocate two chunks of memory (byte arrays in Java)
        // and initialize the first chunk.
        byte[] chunk1 = initialData.getBytes();
        byte[] chunk2 = new byte[chunk1.length];
        
        System.out.println("Step 1 & 2: Allocated two chunks and initialized first chunk.");

        // 3. Copy the contents of the first chunk into the second chunk.
        System.arraycopy(chunk1, 0, chunk2, 0, chunk1.length);
        System.out.println("Step 3: Copied chunk1 to chunk2.");

        // 4. Print the contents of the second chunk.
        System.out.println("Step 4: Contents of chunk2 after copy: " + new String(chunk2));

        // 5. Free the memory allocated to the first chunk.
        // In Java, we make it eligible for garbage collection by nullifying the reference.
        chunk1 = null;
        System.out.println("Step 5: Freed chunk1 (set to null for GC).");

        // 6. Print the contents of the second chunk again.
        // The content should remain as chunk2 is an independent copy.
        System.out.println("Step 6: Contents of chunk2 after freeing chunk1: " + new String(chunk2));

        // 7. Free the memory allocated to the second chunk.
        chunk2 = null;
        System.out.println("Step 7: Freed chunk2 (set to null for GC).");
        System.out.println("----------------------------------------");
    }

    public static void main(String[] args) {
        // 5 test cases
        demonstrateMemoryManagement("Test Case 1: Hello World");
        demonstrateMemoryManagement("Test Case 2: A simple string");
        demonstrateMemoryManagement("Test Case 3: Data");
        demonstrateMemoryManagement("Test Case 4: Another example with numbers 123");
        demonstrateMemoryManagement("Test Case 5: Final test!");
    }
}