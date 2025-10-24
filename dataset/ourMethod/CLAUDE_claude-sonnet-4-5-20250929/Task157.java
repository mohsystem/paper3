
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Task157 {
    private static final int MIN_SIZE = 1;
    private static final int MAX_SIZE = 1_000_000;
    
    /**
     * Dynamically allocates a byte array of the specified size.
     * @param size The size of the array to allocate
     * @return A newly allocated byte array, or null if invalid size
     */
    public static byte[] allocateMemory(int size) {
        if (size < MIN_SIZE || size > MAX_SIZE) {
            System.err.println("Error: Size must be between " + MIN_SIZE + " and " + MAX_SIZE);
            return null;
        }
        
        try {
            byte[] memory = new byte[size];
            Arrays.fill(memory, (byte) 0);
            return memory;
        } catch (OutOfMemoryError e) {
            System.err.println("Error: Failed to allocate memory of size " + size);
            return null;
        }
    }
    
    /**
     * Deallocates memory by zeroing it and clearing the reference.
     * @param memory The memory to deallocate
     * @return null to allow reference clearing
     */
    public static byte[] deallocateMemory(byte[] memory) {
        if (memory != null) {
            Arrays.fill(memory, (byte) 0);
        }
        return null;
    }
    
    /**
     * Writes data to allocated memory with bounds checking.
     * @param memory The allocated memory
     * @param data The data to write
     * @param offset The offset in memory to start writing
     * @return true if successful, false otherwise
     */
    public static boolean writeToMemory(byte[] memory, byte[] data, int offset) {
        if (memory == null || data == null) {
            System.err.println("Error: Memory or data is null");
            return false;
        }
        
        if (offset < 0 || offset > memory.length) {
            System.err.println("Error: Invalid offset");
            return false;
        }
        
        if (data.length > memory.length - offset) {
            System.err.println("Error: Data too large for memory buffer");
            return false;
        }
        
        System.arraycopy(data, 0, memory, offset, data.length);
        return true;
    }
    
    /**
     * Reads data from allocated memory with bounds checking.
     * @param memory The allocated memory
     * @param offset The offset to start reading
     * @param length The number of bytes to read
     * @return The read data, or null if error
     */
    public static byte[] readFromMemory(byte[] memory, int offset, int length) {
        if (memory == null) {
            System.err.println("Error: Memory is null");
            return null;
        }
        
        if (offset < 0 || offset >= memory.length) {
            System.err.println("Error: Invalid offset");
            return null;
        }
        
        if (length < 0 || length > memory.length - offset) {
            System.err.println("Error: Invalid length");
            return null;
        }
        
        byte[] result = new byte[length];
        System.arraycopy(memory, offset, result, 0, length);
        return result;
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Basic allocation and deallocation");
        byte[] mem1 = allocateMemory(100);
        if (mem1 != null) {
            System.out.println("Allocated 100 bytes successfully");
            mem1 = deallocateMemory(mem1);
            System.out.println("Deallocated successfully\\n");
        }
        
        System.out.println("Test Case 2: Write and read operations");
        byte[] mem2 = allocateMemory(50);
        if (mem2 != null) {
            String testData = "Hello, World!";
            byte[] dataBytes = testData.getBytes(StandardCharsets.UTF_8);
            
            if (writeToMemory(mem2, dataBytes, 0)) {
                System.out.println("Written data to memory");
                byte[] readData = readFromMemory(mem2, 0, dataBytes.length);
                if (readData != null) {
                    String readString = new String(readData, StandardCharsets.UTF_8);
                    System.out.println("Read data: " + readString);
                }
            }
            mem2 = deallocateMemory(mem2);
            System.out.println();
        }
        
        System.out.println("Test Case 3: Invalid size allocation");
        byte[] mem3 = allocateMemory(-10);
        if (mem3 == null) {
            System.out.println("Correctly rejected negative size");
        }
        mem3 = allocateMemory(2_000_000);
        if (mem3 == null) {
            System.out.println("Correctly rejected oversized allocation\\n");
        }
        
        System.out.println("Test Case 4: Buffer overflow prevention");
        byte[] mem4 = allocateMemory(10);
        if (mem4 != null) {
            byte[] largeData = new byte[20];
            if (!writeToMemory(mem4, largeData, 0)) {
                System.out.println("Correctly prevented buffer overflow");
            }
            mem4 = deallocateMemory(mem4);
            System.out.println();
        }
        
        System.out.println("Test Case 5: Out of bounds read");
        byte[] mem5 = allocateMemory(20);
        if (mem5 != null) {
            byte[] result = readFromMemory(mem5, 15, 10);
            if (result == null) {
                System.out.println("Correctly prevented out of bounds read");
            }
            mem5 = deallocateMemory(mem5);
        }
    }
}
