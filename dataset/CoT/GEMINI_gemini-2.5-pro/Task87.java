import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;

public class Task87 {

    /**
     * Performs a binary search on a sorted integer array.
     * This implementation assumes the input array is sorted.
     *
     * @param arr    The sorted array to search in.
     * @param target The element to search for.
     * @return The index of the target element, or -1 if not found.
     */
    public static int binarySearch(int[] arr, int target) {
        if (arr == null || arr.length == 0) {
            return -1;
        }

        int low = 0;
        int high = arr.length - 1;

        while (low <= high) {
            // Mitigates integer overflow for large arrays
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
     * Executes binary search, measures its execution time, and logs it to a temporary file.
     *
     * @param arr    The sorted array to search in.
     * @param target The element to search for.
     * @return The index of the target element, or -1 if not found.
     */
    public static int performSearchAndLogTime(int[] arr, int target) {
        long startTime = System.nanoTime();
        int result = binarySearch(arr, target);
        long endTime = System.nanoTime();
        long duration = endTime - startTime;

        Path tempFile = null;
        try {
            // Securely create a temporary file
            tempFile = Files.createTempFile("execution_time_", ".log");
            String logMessage = String.format(
                "Binary search for target %d on array %s took %d nanoseconds. Result index: %d%n",
                target, Arrays.toString(arr), duration, result);
            
            // Use try-with-resources to ensure the writer is closed automatically
            try (BufferedWriter writer = Files.newBufferedWriter(tempFile)) {
                writer.write(logMessage);
            }
            System.out.println("Execution time logged to temporary file: " + tempFile.toAbsolutePath());

        } catch (IOException e) {
            System.err.println("Error writing to temporary file: " + e.getMessage());
        }

        return result;
    }

    public static void main(String[] args) {
        // The array must be sorted for binary search to work correctly.
        int[] sortedArray = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
        
        System.out.println("Running test cases...");

        // Test Case 1: Element in the middle
        int target1 = 23;
        System.out.printf("Searching for %d... Found at index: %d\n\n", target1, performSearchAndLogTime(sortedArray, target1));

        // Test Case 2: Element at the beginning
        int target2 = 2;
        System.out.printf("Searching for %d... Found at index: %d\n\n", target2, performSearchAndLogTime(sortedArray, target2));

        // Test Case 3: Element at the end
        int target3 = 91;
        System.out.printf("Searching for %d... Found at index: %d\n\n", target3, performSearchAndLogTime(sortedArray, target3));

        // Test Case 4: Element not found (smaller than all)
        int target4 = 1;
        System.out.printf("Searching for %d... Found at index: %d\n\n", target4, performSearchAndLogTime(sortedArray, target4));

        // Test Case 5: Element not found (larger than all)
        int target5 = 100;
        System.out.printf("Searching for %d... Found at index: %d\n\n", target5, performSearchAndLogTime(sortedArray, target5));
    }
}