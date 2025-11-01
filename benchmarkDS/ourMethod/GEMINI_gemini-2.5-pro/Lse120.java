import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse120 {

    /**
     * Allocates two byte arrays, copies the content from the first to the second,
     * and prints the content of the second array.
     *
     * @param initialData The string data to be used for the memory chunks.
     */
    public static void manageAndCopyMemory(String initialData) {
        if (initialData == null) {
            System.err.println("Error: Initial data cannot be null.");
            return;
        }

        try {
            // 1. Allocate and populate the first chunk of memory (source).
            // The data is converted to bytes using a standard charset.
            byte[] source = initialData.getBytes(StandardCharsets.UTF_8);

            // 2. Allocate the second chunk of memory (destination).
            // The size is determined by the source to prevent overflow.
            if (source.length > Integer.MAX_VALUE) {
                System.err.println("Error: Source data is too large.");
                return;
            }
            byte[] destination = new byte[source.length];

            // 3. Copy the contents of the first chunk into the second chunk.
            // System.arraycopy is a safe, bounds-checked native method.
            System.arraycopy(source, 0, destination, 0, source.length);

            // 4. Print the contents of the second chunk.
            String copiedString = new String(destination, StandardCharsets.UTF_8);
            System.out.println("Copied data: \"" + copiedString + "\"");

        } catch (OutOfMemoryError e) {
            System.err.println("Error: Failed to allocate memory. " + e.getMessage());
        } catch (Exception e) {
            System.err.println("An unexpected error occurred: " + e.getMessage());
        }
    }

    /**
     * Main method with test cases.
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        System.out.println("--- Test Case 1: Standard string ---");
        manageAndCopyMemory("Hello, World!");
        
        System.out.println("\n--- Test Case 2: Empty string ---");
        manageAndCopyMemory("");

        System.out.println("\n--- Test Case 3: String with numbers and symbols ---");
        manageAndCopyMemory("123!@#$%^&*()_+");

        System.out.println("\n--- Test Case 4: Long string ---");
        manageAndCopyMemory("This is a much longer string to test the memory allocation and copy for a slightly larger chunk of data.");

        System.out.println("\n--- Test Case 5: Null input ---");
        manageAndCopyMemory(null);
    }
}