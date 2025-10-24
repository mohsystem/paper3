
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.PosixFilePermission;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class Task87 {
    
    public static int binarySearch(int[] array, int target) {
        if (array == null) {
            throw new IllegalArgumentException("Array cannot be null");
        }
        
        int left = 0;
        int right = array.length - 1;
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            
            if (array[mid] == target) {
                return mid;
            } else if (array[mid] < target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        return -1;
    }
    
    public static String executeAndMeasure(int[] array, int target) {
        if (array == null) {
            throw new IllegalArgumentException("Array cannot be null");
        }
        
        long startTime = System.nanoTime();
        int result = binarySearch(array, target);
        long endTime = System.nanoTime();
        long executionTime = endTime - startTime;
        
        return String.format("Target: %d, Result: %d, Execution time: %d nanoseconds%n", 
                           target, result, executionTime);
    }
    
    public static void writeExecutionTimeToFile(String content, String baseDir) throws IOException {
        if (content == null || baseDir == null) {
            throw new IllegalArgumentException("Content and baseDir cannot be null");
        }
        
        Path basePath = Paths.get(baseDir).toAbsolutePath().normalize();
        if (!Files.exists(basePath) || !Files.isDirectory(basePath)) {
            throw new IllegalArgumentException("Base directory does not exist or is not a directory");
        }
        
        Path tempFile = Files.createTempFile(basePath, "execution_time_", ".txt");
        
        try {
            Set<PosixFilePermission> perms = new HashSet<>();
            perms.add(PosixFilePermission.OWNER_READ);
            perms.add(PosixFilePermission.OWNER_WRITE);
            try {
                Files.setPosixFilePermissions(tempFile, perms);
            } catch (UnsupportedOperationException e) {
                // Windows does not support POSIX permissions
            }
            
            Files.write(tempFile, content.getBytes(StandardCharsets.UTF_8), 
                       StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING);
            
        } catch (IOException e) {
            try {
                Files.deleteIfExists(tempFile);
            } catch (IOException ignored) {
            }
            throw e;
        }
    }
    
    public static void main(String[] args) {
        try {
            // Test case 1
            int[] arr1 = {1, 3, 5, 7, 9, 11, 13, 15};
            String result1 = executeAndMeasure(arr1, 7);
            System.out.print(result1);
            
            // Test case 2
            int[] arr2 = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20};
            String result2 = executeAndMeasure(arr2, 14);
            System.out.print(result2);
            
            // Test case 3
            int[] arr3 = {10, 20, 30, 40, 50};
            String result3 = executeAndMeasure(arr3, 25);
            System.out.print(result3);
            
            // Test case 4
            int[] arr4 = {5, 10, 15, 20, 25, 30, 35, 40, 45, 50};
            String result4 = executeAndMeasure(arr4, 5);
            System.out.print(result4);
            
            // Test case 5
            int[] arr5 = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
            String result5 = executeAndMeasure(arr5, 1000);
            System.out.print(result5);
            
            String allResults = result1 + result2 + result3 + result4 + result5;
            
            String tmpDir = System.getProperty("java.io.tmpdir");
            writeExecutionTimeToFile(allResults, tmpDir);
            
            System.out.println("Execution times written to temporary file successfully.");
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
