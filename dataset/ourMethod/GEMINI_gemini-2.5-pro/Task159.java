import java.util.Optional;

public class Task159 {

    private final byte[] buffer;

    public Task159(int size) {
        if (size <= 0) {
            throw new IllegalArgumentException("Buffer size must be positive.");
        }
        this.buffer = new byte[size];
        // Populate buffer with some data
        for (int i = 0; i < size; i++) {
            buffer[i] = (byte) (i * 2);
        }
    }

    /**
     * Reads a byte from the buffer at the given index.
     *
     * @param index The index to read from.
     * @return An Optional containing the byte if the index is valid, otherwise an empty Optional.
     */
    public Optional<Byte> readFromBuffer(int index) {
        // Rules#1, Rules#4: Ensure that the index is within the buffer's boundaries.
        if (index >= 0 && index < this.buffer.length) {
            return Optional.of(this.buffer[index]);
        } else {
            return Optional.empty();
        }
    }

    public static void main(String[] args) {
        int bufferSize = 20;
        Task159 memoryManager = new Task159(bufferSize);

        int[] testIndices = {0, 10, 19, -1, 20};

        System.out.println("Testing reads from a buffer of size " + bufferSize + ":");

        for (int i = 0; i < testIndices.length; i++) {
            int index = testIndices[i];
            System.out.print("Test Case " + (i + 1) + ": Reading at index " + index + "... ");
            
            Optional<Byte> result = memoryManager.readFromBuffer(index);
            
            if (result.isPresent()) {
                System.out.println("Success! Value: " + result.get());
            } else {
                System.out.println("Failed! Index is out of bounds.");
            }
        }
    }
}