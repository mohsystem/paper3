import java.util.Arrays;

public class Task159 {

    /**
     * Reads an integer value from a dynamically allocated buffer at a given index.
     *
     * @param buffer The integer array (buffer).
     * @param index  The index from which to read the data.
     * @return The value at the specified index, or -1 if the index is out of bounds.
     */
    public static int readFromBuffer(int[] buffer, int index) {
        if (buffer == null || index < 0 || index >= buffer.length) {
            System.err.println("Error: Index " + index + " is out of bounds.");
            return -1; // Return a sentinel value for an invalid index
        }
        return buffer[index];
    }

    public static void main(String[] args) {
        int bufferSize = 10;

        // 1. Dynamically allocate a memory buffer (an array in Java)
        int[] dynamicBuffer = new int[bufferSize];

        // 2. Initialize the buffer with some data
        for (int i = 0; i < bufferSize; i++) {
            dynamicBuffer[i] = i * 10;
        }
        System.out.println("Buffer created: " + Arrays.toString(dynamicBuffer));
        System.out.println("------------------------------------");

        // 3. Test cases to read data from the buffer
        // Test Case 1: Read from a valid index at the beginning
        int index1 = 0;
        int value1 = readFromBuffer(dynamicBuffer, index1);
        System.out.println("Reading from index " + index1 + ": " + value1);

        // Test Case 2: Read from a valid index in the middle
        int index2 = 5;
        int value2 = readFromBuffer(dynamicBuffer, index2);
        System.out.println("Reading from index " + index2 + ": " + value2);

        // Test Case 3: Read from a valid index at the end
        int index3 = bufferSize - 1;
        int value3 = readFromBuffer(dynamicBuffer, index3);
        System.out.println("Reading from index " + index3 + ": " + value3);

        // Test Case 4: Read from an invalid index (negative)
        int index4 = -1;
        int value4 = readFromBuffer(dynamicBuffer, index4);
        System.out.println("Reading from index " + index4 + ": " + value4);

        // Test Case 5: Read from an invalid index (out of bounds)
        int index5 = bufferSize;
        int value5 = readFromBuffer(dynamicBuffer, index5);
        System.out.println("Reading from index " + index5 + ": " + value5);
        
        // In Java, the garbage collector handles memory deallocation automatically.
    }
}