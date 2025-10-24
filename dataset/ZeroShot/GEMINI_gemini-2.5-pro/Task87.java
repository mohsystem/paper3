import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

public class Task87 {

    /**
     * Performs a binary search on a sorted array of integers.
     *
     * @param arr    The sorted array to search in. Must not be null.
     * @param target The integer to search for.
     * @return The index of the target element, or -1 if it's not found.
     */
    public static int binarySearch(int[] arr, int target) {
        if (arr == null || arr.length == 0) {
            return -1;
        }

        int low = 0;
        int high = arr.length - 1;

        while (low <= high) {
            // Secure way to calculate mid to prevent potential overflow
            int mid = low + (high - low) / 2;

            if (arr[mid] == target) {
                return mid;
            } else if (arr[mid] < target) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        return -1; // Target not found
    }

    /**
     * Measures the execution time of the binary search and writes it to a secure temporary file.
     *
     * @param arr    The sorted array to search in.
     * @param target The integer to search for.
     * @return The index of the target element, or -1 if not found.
     */
    public static int measureAndWriteTime(int[] arr, int target) {
        long startTime = System.nanoTime();
        int result = binarySearch(arr, target);
        long endTime = System.nanoTime();
        long durationNano = endTime - startTime;

        try {
            // Create a secure temporary file
            Path tempFile = Files.createTempFile("search_time_", ".txt");

            // Use try-with-resources to ensure the writer is closed automatically
            try (BufferedWriter writer = new BufferedWriter(new FileWriter(tempFile.toFile()))) {
                String logMessage = "Binary search for target " + target + " took " + durationNano + " nanoseconds.\n";
                writer.write(logMessage);
                System.out.println("Execution time written to temporary file: " + tempFile.toString());
            }

        } catch (IOException e) {
            // Handle potential I/O errors gracefully
            System.err.println("Error writing to temporary file: " + e.getMessage());
        } catch (SecurityException e) {
            System.err.println("Security error creating temporary file: " + e.getMessage());
        }

        return result;
    }

    public static void main(String[] args) {
        // The array must be sorted for binary search to work correctly.
        int[] sortedArray = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
        
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Element in the middle
        int target1 = 23;
        int result1 = measureAndWriteTime(sortedArray, target1);
        System.out.println("Target: " + target1 + ", Found at index: " + result1 + "\n");

        // Test Case 2: Element not present
        int target2 = 15;
        int result2 = measureAndWriteTime(sortedArray, target2);
        System.out.println("Target: " + target2 + ", Found at index: " + result2 + "\n");

        // Test Case 3: First element
        int target3 = 2;
        int result3 = measureAndWriteTime(sortedArray, target3);
        System.out.println("Target: " + target3 + ", Found at index: " + result3 + "\n");

        // Test Case 4: Last element
        int target4 = 91;
        int result4 = measureAndWriteTime(sortedArray, target4);
        System.out.println("Target: " + target4 + ", Found at index: " + result4 + "\n");

        // Test Case 5: Empty array
        int[] emptyArray = {};
        int target5 = 5;
        int result5 = measureAndWriteTime(emptyArray, target5);
        System.out.println("Target: " + target5 + " (in empty array), Found at index: " + result5 + "\n");
    }
}