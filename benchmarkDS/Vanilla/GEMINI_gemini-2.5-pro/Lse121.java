public class Lse121 {

    public static void demonstrateMemory(String data) {
        System.out.println("--- Running test with data: \"" + data + "\" ---");

        // 1. Allocate two chunks of memory (simulated with char arrays)
        char[] chunk1 = data.toCharArray();
        char[] chunk2 = new char[chunk1.length];

        System.out.println("Step 1: Two chunks allocated.");

        // 2. Copy the contents of the first chunk into the second chunk.
        System.arraycopy(chunk1, 0, chunk2, 0, chunk1.length);
        System.out.println("Step 2: Contents of chunk1 copied to chunk2.");

        // 3. Print the contents of the second chunk.
        System.out.print("Step 3: Contents of chunk2 after copy: ");
        System.out.println(new String(chunk2));

        // 4. Free the memory allocated to the first chunk (by making it eligible for garbage collection).
        chunk1 = null;
        // In Java, we can suggest garbage collection, but it's not guaranteed to run immediately.
        // System.gc(); 
        System.out.println("Step 4: chunk1 has been freed (set to null).");


        // 5. Print the contents of the second chunk again.
        // The contents should remain as chunk2 is an independent copy.
        System.out.print("Step 5: Contents of chunk2 after freeing chunk1: ");
        System.out.println(new String(chunk2));

        // 6. Free the memory allocated to the second chunk.
        chunk2 = null;
        System.out.println("Step 6: chunk2 has been freed (set to null).");
        System.out.println("--- Test finished ---\n");
    }

    public static void main(String[] args) {
        // 5 test cases
        demonstrateMemory("Hello World");
        demonstrateMemory("Java Test Case");
        demonstrateMemory("12345");
        demonstrateMemory("Another piece of data.");
        demonstrateMemory(""); // Empty string test
    }
}