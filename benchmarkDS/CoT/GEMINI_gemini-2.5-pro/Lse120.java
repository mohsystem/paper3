class Lse120 {

    /**
     * Allocates two memory chunks (char arrays), copies the content of the first
     * to the second, and then prints the second chunk.
     *
     * @param data The character data to be used for the operation.
     */
    public static void memoryCopyAndPrint(char[] data) {
        if (data == null) {
            System.out.println("Input data cannot be null.");
            return;
        }

        int size = data.length;

        // 1. Allocate two chunks of memory (char arrays in Java)
        char[] sourceChunk = new char[size];
        char[] destinationChunk = new char[size];

        // For demonstration, we populate the first chunk with the input data.
        // System.arraycopy is a safe way to copy array contents.
        System.arraycopy(data, 0, sourceChunk, 0, size);

        // 2. Copy the contents of the first chunk into the second chunk.
        // This method is safe as it checks for array bounds internally.
        System.arraycopy(sourceChunk, 0, destinationChunk, 0, size);

        // 3. Print the contents of the second chunk.
        System.out.print("Contents of the second chunk: ");
        System.out.println(destinationChunk);
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Regular string
        memoryCopyAndPrint("Hello, Java!".toCharArray());

        // Test Case 2: String with numbers and symbols
        memoryCopyAndPrint("123 Test Cases!@#".toCharArray());

        // Test Case 3: Longer string
        memoryCopyAndPrint("This is a longer test string to check allocation.".toCharArray());

        // Test Case 4: Single character
        memoryCopyAndPrint("A".toCharArray());

        // Test Case 5: Empty string
        memoryCopyAndPrint("".toCharArray());
    }
}