
import java.util.Scanner;

public class Task159 {
    private static int[] buffer;
    private static int bufferSize;
    
    public static void initializeBuffer(int size, int[] initialData) {
        bufferSize = size;
        buffer = new int[size];
        if (initialData != null) {
            for (int i = 0; i < Math.min(size, initialData.length); i++) {
                buffer[i] = initialData[i];
            }
        }
    }
    
    public static Integer readFromBuffer(int index) {
        if (buffer == null) {
            System.out.println("Error: Buffer not initialized");
            return null;
        }
        if (index < 0 || index >= bufferSize) {
            System.out.println("Error: Index out of bounds");
            return null;
        }
        return buffer[index];
    }
    
    public static void deallocateBuffer() {
        buffer = null;
        bufferSize = 0;
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic read operation
        System.out.println("Test Case 1: Basic read operation");
        int[] data1 = {10, 20, 30, 40, 50};
        initializeBuffer(5, data1);
        Integer result = readFromBuffer(2);
        System.out.println("Read from index 2: " + result);
        deallocateBuffer();
        System.out.println();
        
        // Test case 2: Read from first index
        System.out.println("Test Case 2: Read from first index");
        int[] data2 = {100, 200, 300};
        initializeBuffer(3, data2);
        result = readFromBuffer(0);
        System.out.println("Read from index 0: " + result);
        deallocateBuffer();
        System.out.println();
        
        // Test case 3: Read from last index
        System.out.println("Test Case 3: Read from last index");
        int[] data3 = {5, 15, 25, 35, 45, 55};
        initializeBuffer(6, data3);
        result = readFromBuffer(5);
        System.out.println("Read from index 5: " + result);
        deallocateBuffer();
        System.out.println();
        
        // Test case 4: Invalid index (negative)
        System.out.println("Test Case 4: Invalid index (negative)");
        int[] data4 = {1, 2, 3, 4};
        initializeBuffer(4, data4);
        result = readFromBuffer(-1);
        System.out.println("Result: " + result);
        deallocateBuffer();
        System.out.println();
        
        // Test case 5: Invalid index (out of bounds)
        System.out.println("Test Case 5: Invalid index (out of bounds)");
        int[] data5 = {7, 14, 21, 28};
        initializeBuffer(4, data5);
        result = readFromBuffer(10);
        System.out.println("Result: " + result);
        deallocateBuffer();
        System.out.println();
    }
}
