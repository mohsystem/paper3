
import java.nio.charset.StandardCharsets;
import java.util.Scanner;

public class Task159 {
    private static final int BUFFER_SIZE = 100;
    private static final int MIN_INDEX = 0;
    
    public static class BufferReader {
        private final byte[] buffer;
        private final int size;
        
        public BufferReader(int size) {
            if (size <= 0 || size > 1000000) {
                throw new IllegalArgumentException("Buffer size must be between 1 and 1000000");
            }
            this.size = size;
            this.buffer = new byte[size];
            initializeBuffer();
        }
        
        private void initializeBuffer() {
            String data = "Hello, this is sample data in the buffer for testing purposes.";
            byte[] dataBytes = data.getBytes(StandardCharsets.UTF_8);
            int copyLength = Math.min(dataBytes.length, buffer.length);
            System.arraycopy(dataBytes, 0, buffer, 0, copyLength);
        }
        
        public byte readAtIndex(int index) {
            if (index < MIN_INDEX || index >= size) {
                throw new IndexOutOfBoundsException(
                    String.format("Index %d is out of bounds. Valid range: [%d, %d)", 
                    index, MIN_INDEX, size)
                );
            }
            return buffer[index];
        }
        
        public int getSize() {
            return size;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Valid index at start ===");
        testReadAtIndex(BUFFER_SIZE, 0);
        
        System.out.println("\\n=== Test Case 2: Valid index in middle ===");
        testReadAtIndex(BUFFER_SIZE, 10);
        
        System.out.println("\\n=== Test Case 3: Valid index at end ===");
        testReadAtIndex(BUFFER_SIZE, BUFFER_SIZE - 1);
        
        System.out.println("\\n=== Test Case 4: Invalid negative index ===");
        testReadAtIndex(BUFFER_SIZE, -1);
        
        System.out.println("\\n=== Test Case 5: Invalid index beyond buffer ===");
        testReadAtIndex(BUFFER_SIZE, BUFFER_SIZE);
    }
    
    private static void testReadAtIndex(int bufferSize, int index) {
        try {
            BufferReader reader = new BufferReader(bufferSize);
            byte value = reader.readAtIndex(index);
            System.out.printf("Successfully read byte at index %d: %d (char: '%c')%n", 
                index, value, (char) value);
        } catch (IndexOutOfBoundsException e) {
            System.out.printf("Error reading at index %d: %s%n", index, e.getMessage());
        } catch (IllegalArgumentException e) {
            System.out.printf("Error creating buffer: %s%n", e.getMessage());
        }
    }
}
