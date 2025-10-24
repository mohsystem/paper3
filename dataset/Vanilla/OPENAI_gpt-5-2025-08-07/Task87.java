import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.charset.StandardCharsets;

public class Task87 {
    public static int binarySearch(int[] arr, int target) {
        int lo = 0, hi = arr.length - 1;
        while (lo <= hi) {
            int mid = lo + ((hi - lo) >>> 1);
            int val = arr[mid];
            if (val == target) return mid;
            if (val < target) lo = mid + 1;
            else hi = mid - 1;
        }
        return -1;
    }

    public static void main(String[] args) throws Exception {
        long start = System.nanoTime();

        int idx1 = binarySearch(new int[]{1, 3, 5, 7, 9}, 7);
        int idx2 = binarySearch(new int[]{1, 3, 5, 7, 9}, 2);
        int idx3 = binarySearch(new int[]{}, 1);
        int idx4 = binarySearch(new int[]{5}, 5);
        int idx5 = binarySearch(new int[]{-10, -3, 0, 5, 5, 9, 12}, 5);

        System.out.println("Test1 index: " + idx1);
        System.out.println("Test2 index: " + idx2);
        System.out.println("Test3 index: " + idx3);
        System.out.println("Test4 index: " + idx4);
        System.out.println("Test5 index: " + idx5);

        long elapsed = System.nanoTime() - start;
        Path temp = Files.createTempFile("Task87_", ".tmp");
        String content = "elapsed_nanos=" + elapsed + System.lineSeparator() +
                         "elapsed_millis=" + (elapsed / 1_000_000.0) + System.lineSeparator();
        Files.write(temp, content.getBytes(StandardCharsets.UTF_8));
        System.out.println("Wrote timing to: " + temp.toAbsolutePath());
    }
}