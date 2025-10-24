
import java.io.*;
import java.nio.file.*;
import java.util.Arrays;

public class Task87 {
    
    public static int binarySearch(int[] arr, int target) {
        if (arr == null) {
            return -1;
        }
        
        int left = 0;
        int right = arr.length - 1;
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            
            if (arr[mid] == target) {
                return mid;
            } else if (arr[mid] < target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        return -1;
    }
    
    public static void writeExecutionTime(long executionTime) {
        try {
            Path tempFile = Files.createTempFile("binary_search_execution_", ".txt");
            String content = "Binary Search Execution Time: " + executionTime + " nanoseconds\\n";
            Files.write(tempFile, content.getBytes(), StandardOpenOption.WRITE);
            System.out.println("Execution time written to: " + tempFile.toString());
        } catch (IOException e) {
            System.err.println("Error writing to temporary file: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Target found in middle
        int[] arr1 = {1, 3, 5, 7, 9, 11, 13, 15};
        int target1 = 7;
        long startTime1 = System.nanoTime();
        int result1 = binarySearch(arr1, target1);
        long endTime1 = System.nanoTime();
        long executionTime1 = endTime1 - startTime1;
        System.out.println("Test 1: Array: " + Arrays.toString(arr1) + ", Target: " + target1 + ", Result: " + result1);
        writeExecutionTime(executionTime1);
        
        // Test Case 2: Target found at beginning
        int[] arr2 = {2, 4, 6, 8, 10, 12, 14};
        int target2 = 2;
        long startTime2 = System.nanoTime();
        int result2 = binarySearch(arr2, target2);
        long endTime2 = System.nanoTime();
        long executionTime2 = endTime2 - startTime2;
        System.out.println("Test 2: Array: " + Arrays.toString(arr2) + ", Target: " + target2 + ", Result: " + result2);
        writeExecutionTime(executionTime2);
        
        // Test Case 3: Target found at end
        int[] arr3 = {5, 10, 15, 20, 25, 30};
        int target3 = 30;
        long startTime3 = System.nanoTime();
        int result3 = binarySearch(arr3, target3);
        long endTime3 = System.nanoTime();
        long executionTime3 = endTime3 - startTime3;
        System.out.println("Test 3: Array: " + Arrays.toString(arr3) + ", Target: " + target3 + ", Result: " + result3);
        writeExecutionTime(executionTime3);
        
        // Test Case 4: Target not found
        int[] arr4 = {1, 2, 3, 4, 5};
        int target4 = 10;
        long startTime4 = System.nanoTime();
        int result4 = binarySearch(arr4, target4);
        long endTime4 = System.nanoTime();
        long executionTime4 = endTime4 - startTime4;
        System.out.println("Test 4: Array: " + Arrays.toString(arr4) + ", Target: " + target4 + ", Result: " + result4);
        writeExecutionTime(executionTime4);
        
        // Test Case 5: Empty array
        int[] arr5 = {};
        int target5 = 5;
        long startTime5 = System.nanoTime();
        int result5 = binarySearch(arr5, target5);
        long endTime5 = System.nanoTime();
        long executionTime5 = endTime5 - startTime5;
        System.out.println("Test 5: Array: " + Arrays.toString(arr5) + ", Target: " + target5 + ", Result: " + result5);
        writeExecutionTime(executionTime5);
    }
}
