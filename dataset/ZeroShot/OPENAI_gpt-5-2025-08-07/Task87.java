import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.util.Arrays;
import java.util.EnumSet;
import java.util.Set;

public class Task87 {

    public static int binarySearch(int[] arr, int target) {
        if (arr == null) return -1;
        int left = 0;
        int right = arr.length - 1;
        while (left <= right) {
            int mid = left + ((right - left) >>> 1);
            int val = arr[mid];
            if (val == target) return mid;
            if (val < target) left = mid + 1;
            else right = mid - 1;
        }
        return -1;
    }

    public static Path writeExecutionTimeToTemp(long nanos) throws IOException {
        Path temp = Files.createTempFile("task87_bsearch_", ".tmp");
        try {
            Set<PosixFilePermission> perms = EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);
            Files.setPosixFilePermissions(temp, perms);
        } catch (UnsupportedOperationException ignored) {
            // Non-POSIX file system; ignore permission setting.
        }
        try (BufferedWriter writer = Files.newBufferedWriter(temp, StandardCharsets.UTF_8, StandardOpenOption.TRUNCATE_EXISTING)) {
            writer.write(Long.toString(nanos));
            writer.newLine();
        }
        temp.toFile().deleteOnExit();
        return temp;
    }

    public static void main(String[] args) {
        long start = System.nanoTime();

        // Test case 1
        int[] a1 = {1, 3, 5, 7, 9};
        System.out.println("Test 1: " + binarySearch(a1, 7)); // expected 3

        // Test case 2
        int[] a2 = {};
        System.out.println("Test 2: " + binarySearch(a2, 1)); // expected -1

        // Test case 3
        int[] a3 = {-10, -3, 0, 0, 5, 8};
        System.out.println("Test 3: " + binarySearch(a3, 0)); // expected 2 or 3

        // Test case 4
        int[] a4 = {1};
        System.out.println("Test 4: " + binarySearch(a4, 1)); // expected 0

        // Test case 5
        int[] a5 = {2, 4, 6, 8, 10, 12, 14, 16};
        System.out.println("Test 5: " + binarySearch(a5, 15)); // expected -1

        long end = System.nanoTime();
        long elapsed = end - start;

        try {
            Path p = writeExecutionTimeToTemp(elapsed);
            System.out.println("Execution time (ns) written to: " + p.toAbsolutePath());
        } catch (IOException e) {
            System.err.println("Failed to write execution time: " + e.getMessage());
        }
    }
}