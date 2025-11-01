import java.util.Arrays;

public class Lse122 {

    /**
     * Allocates two byte arrays, copies content from the first to the second,
     * and then marks them for garbage collection.
     * @param testCaseData The data to initialize the first chunk with.
     */
    public static void manageMemory(String testCaseData) {
        final int SIZE = 10;
        System.out.println("--- Starting Memory Management Demo ---");

        // 1. Allocate two chunks of memory each of size 10.
        // In Java, this is done by creating byte arrays. The JVM handles actual memory allocation.
        // Memory allocation failure will throw an OutOfMemoryError.
        byte[] chunk1 = new byte[SIZE];
        byte[] chunk2 = new byte[SIZE];
        System.out.println("1. Allocated two memory chunks of size " + SIZE);

        // Initialize the first chunk to make the copy operation meaningful.
        byte[] sourceData = testCaseData.getBytes();
        // Use Math.min to prevent ArrayStoreException if sourceData is larger than SIZE.
        int lengthToCopy = Math.min(sourceData.length, SIZE);
        System.arraycopy(sourceData, 0, chunk1, 0, lengthToCopy);
        System.out.println("   Initialized chunk1: " + Arrays.toString(chunk1));
        System.out.println("   (chunk2 is currently empty: " + Arrays.toString(chunk2) + ")");

        // 2. Copy the contents of the first chunk into the second chunk.
        // System.arraycopy is a safe, built-in method that includes bounds checking.
        System.arraycopy(chunk1, 0, chunk2, 0, SIZE);
        System.out.println("2. Copied chunk1 to chunk2.");
        System.out.println("   Content of chunk2 after copy: " + Arrays.toString(chunk2));

        // 3. Free the first chunk.
        // In Java, "freeing" means making the object eligible for garbage collection
        // by removing all references to it. Here, we set the reference to null.
        chunk1 = null;
        System.out.println("3. Freed first chunk (now eligible for garbage collection).");
        
        // 4. Free the second chunk.
        chunk2 = null;
        System.out.println("4. Freed second chunk (now eligible for garbage collection).");
        System.out.println("--- Demo Finished ---\n");
    }

    public static void main(String[] args) {
        System.out.println(">>> Running 5 Test Cases for Memory Management in Java <<<");
        for (int i = 1; i <= 5; i++) {
            System.out.println(">>> Test Case #" + i + " <<<");
            // Use different data for each test case to show the process is repeatable.
            String data = "Data " + i;
            manageMemory(data);
        }
    }
}