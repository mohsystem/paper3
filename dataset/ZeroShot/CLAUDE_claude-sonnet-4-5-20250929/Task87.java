
import java.io.*;
import java.nio.file.*;
import java.util.Arrays;

public class Task87 {
    public static int binarySearch(int[] arr, int target) {
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
            File tempFile = File.createTempFile("execution_time_", ".txt");
            FileWriter writer = new FileWriter(tempFile);
            writer.write("Binary Search Execution Time: " + executionTime + " nanoseconds\\n");
            writer.close();
            System.out.println("Execution time written to: " + tempFile.getAbsolutePath());
        } catch (IOException e) {
            System.err.println("Error writing to file: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] arr1 = {1, 3, 5, 7, 9, 11, 13, 15};
        int target1 = 7;
        long startTime1 = System.nanoTime();
        int result1 = binarySearch(arr1, target1);
        long endTime1 = System.nanoTime();
        long executionTime1 = endTime1 - startTime1;
        System.out.println("Test 1: Array: " + Arrays.toString(arr1) + ", Target: " + target1 + ", Result: " + result1);
        writeExecutionTime(executionTime1);
        
        // Test case 2
        int[] arr2 = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20};
        int target2 = 12;
        long startTime2 = System.nanoTime();
        int result2 = binarySearch(arr2, target2);
        long endTime2 = System.nanoTime();
        long executionTime2 = endTime2 - startTime2;
        System.out.println("Test 2: Array: " + Arrays.toString(arr2) + ", Target: " + target2 + ", Result: " + result2);
        writeExecutionTime(executionTime2);
        
        // Test case 3
        int[] arr3 = {1, 2, 3, 4, 5};
        int target3 = 6;
        long startTime3 = System.nanoTime();
        int result3 = binarySearch(arr3, target3);
        long endTime3 = System.nanoTime();
        long executionTime3 = endTime3 - startTime3;
        System.out.println("Test 3: Array: " + Arrays.toString(arr3) + ", Target: " + target3 + ", Result: " + result3);
        writeExecutionTime(executionTime3);
        
        // Test case 4
        int[] arr4 = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        int target4 = 10;
        long startTime4 = System.nanoTime();
        int result4 = binarySearch(arr4, target4);
        long endTime4 = System.nanoTime();
        long executionTime4 = endTime4 - startTime4;
        System.out.println("Test 4: Array: " + Arrays.toString(arr4) + ", Target: " + target4 + ", Result: " + result4);
        writeExecutionTime(executionTime4);
        
        // Test case 5
        int[] arr5 = {5, 15, 25, 35, 45, 55};
        int target5 = 55;
        long startTime5 = System.nanoTime();
        int result5 = binarySearch(arr5, target5);
        long endTime5 = System.nanoTime();
        long executionTime5 = endTime5 - startTime5;
        System.out.println("Test 5: Array: " + Arrays.toString(arr5) + ", Target: " + target5 + ", Result: " + result5);
        writeExecutionTime(executionTime5);
    }
}
