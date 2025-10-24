import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileSystems;
import java.nio.file.FileVisitOption;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Arrays;

public class Task87 {

    public static int binarySearch(int[] arr, int target) {
        if (arr == null) {
            return -1;
        }
        int left = 0;
        int right = arr.length - 1;
        while (left <= right) {
            int mid = left + ((right - left) >>> 1);
            int val = arr[mid];
            if (val == target) {
                return mid;
            } else if (val < target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        return -1;
    }

    private static String sanitizeLabel(String label) {
        String src = (label == null) ? "" : label;
        if (src.length() > 128) {
            src = src.substring(0, 128);
        }
        return src.replaceAll("[^A-Za-z0-9 _\\-]", "_");
    }

    public static String writeExecutionTimeToTempFile(long nanos, String label) throws IOException {
        String safeLabel = sanitizeLabel(label);
        Path tempPath;
        // Attempt to set strict POSIX permissions where supported
        try {
            boolean posixSupported = Files.getFileAttributeView(FileSystems.getDefault().getPath(System.getProperty("java.io.tmpdir")), PosixFileAttributeView.class) != null;
            if (posixSupported) {
                tempPath = Files.createTempFile("task87_", ".tmp",
                        PosixFilePermissions.asFileAttribute(PosixFilePermissions.fromString("rw-------")));
            } else {
                tempPath = Files.createTempFile("task87_", ".tmp");
                // Best-effort: on non-POSIX, fallback without explicit permissions
            }
        } catch (IOException e) {
            // Fallback if any error occurs
            tempPath = Files.createTempFile("task87_", ".tmp");
        }

        if (Files.isSymbolicLink(tempPath)) {
            Files.deleteIfExists(tempPath);
            throw new IOException("Refusing to write to a symbolic link.");
        }

        String content = "label=" + safeLabel + "\nexecution_time_ns=" + nanos + "\n";
        byte[] bytes = content.getBytes(StandardCharsets.UTF_8);

        try (FileChannel channel = FileChannel.open(tempPath, StandardOpenOption.WRITE);
             FileLock lock = channel.lock()) {
            channel.position(0);
            channel.truncate(0);
            channel.write(ByteBuffer.wrap(bytes));
            channel.force(true);
        }

        return tempPath.toAbsolutePath().toString();
    }

    public static void main(String[] args) {
        int[][] arrays = new int[][]{
                new int[]{1, 3, 5, 7, 9},
                new int[]{2, 4, 6, 8, 10},
                new int[]{0},
                new int[]{-10, -5, 0, 5, 10, 15},
                new int[]{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
        };
        int[] targets = new int[]{7, 1, 0, 15, 11};

        long start = System.nanoTime();
        int[] results = new int[targets.length];
        for (int i = 0; i < targets.length; i++) {
            results[i] = binarySearch(arrays[i], targets[i]);
        }
        long end = System.nanoTime();
        long totalNanos = end - start;

        try {
            String path = writeExecutionTimeToTempFile(totalNanos, "total_run");
            System.out.println("Binary search results (indices): " + Arrays.toString(results));
            System.out.println("Execution time written to: " + path);
        } catch (IOException e) {
            System.err.println("Failed to write execution time: " + e.getMessage());
        }
    }
}