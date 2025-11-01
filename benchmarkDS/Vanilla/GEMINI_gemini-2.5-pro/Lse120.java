import java.util.Arrays;

public class Lse120 {

    /**
     * Allocates two memory chunks (byte arrays), copies the content of the first
     * to the second, and prints the second.
     * @param sourceData The data to initialize the first chunk with.
     */
    public static void allocateCopyAndPrint(byte[] sourceData) {
        if (sourceData == null) {
            System.out.println("Source data is null.");
            return;
        }

        // 1. Allocate the first chunk of memory and copy source data into it.
        // In Java, creating an array allocates memory on the heap.
        byte[] firstChunk = new byte[sourceData.length];
        System.arraycopy(sourceData, 0, firstChunk, 0, sourceData.length);

        // 2. Allocate the second chunk of memory.
        byte[] secondChunk = new byte[firstChunk.length];

        // 3. Copy the contents of the first chunk into the second chunk.
        System.arraycopy(firstChunk, 0, secondChunk, 0, firstChunk.length);

        // 4. Print the contents of the second chunk.
        // Memory is automatically managed (garbage collected) in Java.
        System.out.println("Contents of the second chunk: " + Arrays.toString(secondChunk));
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Regular byte array
        System.out.println("\nTest Case 1:");
        allocateCopyAndPrint(new byte[]{10, 20, 30, 40, 50});

        // Test Case 2: Byte array from a string (ASCII values)
        System.out.println("\nTest Case 2:");
        allocateCopyAndPrint("Hello".getBytes());

        // Test Case 3: Empty array
        System.out.println("\nTest Case 3:");
        allocateCopyAndPrint(new byte[]{});

        // Test Case 4: Single element array
        System.out.println("\nTest Case 4:");
        allocateCopyAndPrint(new byte[]{127});

        // Test Case 5: Array with negative values
        System.out.println("\nTest Case 5:");
        allocateCopyAndPrint(new byte[]{-1, -128, 0, 1, 2});
    }
}