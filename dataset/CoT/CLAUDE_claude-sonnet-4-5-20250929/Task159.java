
import java.util.Scanner;
import java.util.InputMismatchException;

public class Task159 {
    private static final int BUFFER_SIZE = 100;
    
    public static int readFromBuffer(int[] buffer, int index) {
        if (buffer == null) {
            throw new IllegalArgumentException("Buffer cannot be null");
        }
        if (index < 0 || index >= buffer.length) {
            throw new IndexOutOfBoundsException("Index out of bounds: " + index);
        }
        return buffer[index];
    }
    
    public static int[] allocateBuffer(int size) {
        if (size <= 0 || size > 1000000) {
            throw new IllegalArgumentException("Invalid buffer size");
        }
        return new int[size];
    }
    
    public static void initializeBuffer(int[] buffer) {
        for (int i = 0; i < buffer.length; i++) {
            buffer[i] = i * 10;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure Memory Buffer Access Test Cases ===\\n");
        
        // Test Case 1: Normal valid access
        try {
            int[] buffer1 = allocateBuffer(BUFFER_SIZE);
            initializeBuffer(buffer1);
            int result = readFromBuffer(buffer1, 50);
            System.out.println("Test 1 - Valid access at index 50: " + result);
        } catch (Exception e) {
            System.out.println("Test 1 - Error: " + e.getMessage());
        }
        
        // Test Case 2: Access at boundary (first element)
        try {
            int[] buffer2 = allocateBuffer(BUFFER_SIZE);
            initializeBuffer(buffer2);
            int result = readFromBuffer(buffer2, 0);
            System.out.println("Test 2 - Valid access at index 0: " + result);
        } catch (Exception e) {
            System.out.println("Test 2 - Error: " + e.getMessage());
        }
        
        // Test Case 3: Access at boundary (last element)
        try {
            int[] buffer3 = allocateBuffer(BUFFER_SIZE);
            initializeBuffer(buffer3);
            int result = readFromBuffer(buffer3, BUFFER_SIZE - 1);
            System.out.println("Test 3 - Valid access at index " + (BUFFER_SIZE - 1) + ": " + result);
        } catch (Exception e) {
            System.out.println("Test 3 - Error: " + e.getMessage());
        }
        
        // Test Case 4: Invalid access (negative index)
        try {
            int[] buffer4 = allocateBuffer(BUFFER_SIZE);
            initializeBuffer(buffer4);
            int result = readFromBuffer(buffer4, -5);
            System.out.println("Test 4 - Access at index -5: " + result);
        } catch (Exception e) {
            System.out.println("Test 4 - Error (Expected): " + e.getMessage());
        }
        
        // Test Case 5: Invalid access (out of bounds)
        try {
            int[] buffer5 = allocateBuffer(BUFFER_SIZE);
            initializeBuffer(buffer5);
            int result = readFromBuffer(buffer5, 150);
            System.out.println("Test 5 - Access at index 150: " + result);
        } catch (Exception e) {
            System.out.println("Test 5 - Error (Expected): " + e.getMessage());
        }
    }
}
