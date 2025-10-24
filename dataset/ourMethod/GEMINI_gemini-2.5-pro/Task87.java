import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Task87 {

    /**
     * Performs a binary search on a sorted integer array.
     *
     * @param arr    the sorted array to search in. Must not be null.
     * @param target the value to search for.
     * @return the index of the target, or -1 if not found.
     */
    public static int binarySearch(int[] arr, int target) {
        int low = 0;
        int high = arr.length - 1;

        while (low <= high) {
            // Use `low + (high - low) / 2` to prevent potential overflow
            // if `low + high` exceeds Integer.MAX_VALUE.
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

    public static void main(String[] args) {
        int[] sortedArray = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
        int[] testTargets = {23, 91, 2, 15, 100};

        Path tempFile = null;
        try {
            // Securely create a temporary file.
            tempFile = Files.createTempFile("task87_java_times_", ".txt");
            System.out.println("Execution times will be written to: " + tempFile.toAbsolutePath());

            // Use try-with-resources to ensure the writer is closed automatically.
            try (BufferedWriter writer = Files.newBufferedWriter(tempFile, StandardCharsets.UTF_8)) {
                for (int target : testTargets) {
                    long startTime = System.nanoTime();
                    int result = binarySearch(sortedArray, target);
                    long endTime = System.nanoTime();
                    long duration = endTime - startTime;

                    System.out.println("Array: " + Arrays.toString(sortedArray));
                    System.out.println("Searching for: " + target);
                    if (result != -1) {
                        System.out.println("Found at index: " + result);
                    } else {
                        System.out.println("Not found.");
                    }
                    System.out.println("Execution time: " + duration + " nanoseconds.\n");

                    writer.write("Target: " + target + ", Time (ns): " + duration + "\n");
                }
            }
        } catch (IOException e) {
            System.err.println("An error occurred while handling the temporary file.");
            e.printStackTrace();
        } catch (SecurityException e) {
            System.err.println("Security manager prevents creating a temporary file.");
            e.printStackTrace();
        }
    }
}