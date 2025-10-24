import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Arrays;

public class Task87 {

    /**
     * Performs binary search on a sorted integer array.
     *
     * @param arr    The sorted array to search in.
     * @param target The element to search for.
     * @return The index of the target element, or -1 if not found.
     */
    public static int binarySearch(int[] arr, int target) {
        int left = 0;
        int right = arr.length - 1;

        while (left <= right) {
            int mid = left + (right - left) / 2;

            if (arr[mid] == target) {
                return mid; // Element found
            }

            if (arr[mid] < target) {
                left = mid + 1; // Search in the right half
            } else {
                right = mid - 1; // Search in the left half
            }
        }
        return -1; // Element not found
    }

    /**
     * Writes the execution time to a temporary log file.
     *
     * @param duration The execution time in nanoseconds.
     * @param target   The target value that was searched for.
     */
    public static void writeExecutionTime(long duration, int target) {
        // Using a fixed filename for simplicity, acts as a temporary log.
        String tempFileName = "java_execution_log.tmp";
        try (PrintWriter writer = new PrintWriter(new FileWriter(tempFileName, true))) {
            writer.println("Target: " + target + ", Execution Time: " + duration + " ns");
        } catch (IOException e) {
            System.err.println("Error writing to temporary file: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // The array must be sorted for binary search to work.
        int[] sortedArray = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
        int[] testCases = {23, 91, 2, 15, 100}; // Targets to search for

        System.out.println("Starting Java Binary Search Test Cases...");
        System.out.println("Array: " + Arrays.toString(sortedArray));

        for (int target : testCases) {
            System.out.println("----------------------------------------");
            long startTime = System.nanoTime();
            int result = binarySearch(sortedArray, target);
            long endTime = System.nanoTime();
            long duration = endTime - startTime;

            System.out.println("Searching for: " + target);
            if (result != -1) {
                System.out.println("Element found at index: " + result);
            } else {
                System.out.println("Element not found.");
            }
            System.out.println("Execution time: " + duration + " nanoseconds.");

            // Write the execution time to a temporary file
            writeExecutionTime(duration, target);
        }
        System.out.println("----------------------------------------");
        System.out.println("All execution times have been logged to java_execution_log.tmp");
    }
}