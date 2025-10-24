import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.util.Arrays;

public class Task87 {

    public static int binarySearch(int[] arr, int target) {
        if (arr == null || arr.length == 0) return -1;
        int low = 0;
        int high = arr.length - 1;
        while (low <= high) {
            int mid = low + ((high - low) >>> 1);
            int val = arr[mid];
            if (val == target) {
                return mid;
            } else if (val < target) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        return -1;
    }

    public static String measureAndWriteTime(int[] arr, int target) throws IOException {
        long start = System.nanoTime();
        int idx = binarySearch(arr, target);
        long elapsed = System.nanoTime() - start;

        Path temp = Files.createTempFile("task87_time_", ".txt");
        String content = "elapsed_nanos=" + elapsed + ", index=" + idx + System.lineSeparator();
        Files.write(temp, content.getBytes(StandardCharsets.UTF_8),
                StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING);
        return temp.toAbsolutePath().toString();
    }

    public static void main(String[] args) {
        try {
            int[][] tests = new int[5][];
            int[] targets = new int[5];

            tests[0] = new int[]{1, 3, 5, 7, 9}; targets[0] = 7;
            tests[1] = new int[]{2, 4, 6, 8, 10}; targets[1] = 5;
            tests[2] = new int[]{}; targets[2] = 1;
            tests[3] = new int[]{-10, -5, 0, 5, 10}; targets[3] = -10;
            tests[4] = new int[100000]; targets[4] = 12345;
            for (int i = 0; i < tests[4].length; i++) tests[4][i] = i;

            for (int i = 0; i < 5; i++) {
                int idx = binarySearch(tests[i], targets[i]);
                String path = measureAndWriteTime(tests[i], targets[i]);
                System.out.println("Test " + (i + 1) + ": target=" + targets[i] +
                        ", index=" + idx + ", timeFile=" + path);
            }
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}