import java.util.Optional;

public class Task159 {

    /**
     * Reads data from a buffer at a given index.
     *
     * @param buffer The dynamically allocated buffer (represented by an array).
     * @param size The size of the buffer.
     * @param index The index from which to read data.
     * @return An Optional containing the data at the index if valid, otherwise an empty Optional.
     */
    public static Optional<Integer> readFromBuffer(int[] buffer, int size, int index) {
        // Security: Check if the index is within the valid bounds [0, size-1].
        // This prevents ArrayIndexOutOfBoundsException.
        if (index >= 0 && index < size) {
            return Optional.of(buffer[index]);
        } else {
            // Index is out of bounds, return an empty Optional to indicate failure.
            return Optional.empty();
        }
    }

    public static void main(String[] args) {
        final int BUFFER_SIZE = 10;
        // 1. Dynamically allocate a memory buffer (in Java, arrays are dynamically allocated).
        int[] buffer = new int[BUFFER_SIZE];

        // 2. Fill the buffer with some data.
        for (int i = 0; i < BUFFER_SIZE; i++) {
            buffer[i] = i * 10; // e.g., 0, 10, 20, ...
        }

        System.out.println("Java Test Cases:");
        
        // 3. Test cases to read from the buffer.
        int[] testIndices = {3, 0, 9, 10, -1};

        for (int i = 0; i < testIndices.length; i++) {
            int index = testIndices[i];
            System.out.print("Test " + (i + 1) + ": Reading at index " + index + "... ");
            
            Optional<Integer> result = readFromBuffer(buffer, BUFFER_SIZE, index);

            if (result.isPresent()) {
                System.out.println("Success! Value: " + result.get());
            } else {
                System.out.println("Error: Index is out of bounds.");
            }
        }
    }
}