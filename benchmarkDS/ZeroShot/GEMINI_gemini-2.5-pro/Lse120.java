public class Lse120 {

    /**
     * Allocates two memory chunks (as byte arrays), securely copies content from the
     * first to the second, and returns the content of the second chunk as a String.
     *
     * @param content The string content to be copied.
     * @return A new String containing the copied content.
     */
    public static String secureMemoryCopy(String content) {
        if (content == null) {
            return null;
        }

        // 1. The first "chunk of memory" is the byte array from the input string.
        byte[] source = content.getBytes();
        int size = source.length;

        // 1. Allocate the second chunk of memory.
        byte[] destination = new byte[size];

        // 2. Securely copy the contents. System.arraycopy performs bounds checking
        // automatically, preventing buffer overflows. It will throw an
        // IndexOutOfBoundsException if the copy is not possible.
        System.arraycopy(source, 0, destination, 0, size);

        // 3. Return the contents of the second chunk as a new String.
        // Memory management is handled by Java's garbage collector.
        return new String(destination);
    }

    public static void main(String[] args) {
        String[] testCases = {
            "Hello, secure world!",
            "This is a test case with various characters 1@#$%.",
            "Short",
            "", // Empty string test case
            "This is a significantly longer string to ensure that the memory allocation and copying process works correctly with a larger amount of data."
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            String original = testCases[i];
            System.out.println("Original:    \"" + original + "\"");
            
            String copiedContent = secureMemoryCopy(original);
            
            System.out.println("Copied:      \"" + copiedContent + "\"");
            System.out.println("Content matches: " + original.equals(copiedContent));
            System.out.println();
        }
    }
}