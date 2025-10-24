
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import java.util.stream.Collectors;

public class Task94 {
    private static final int MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    private static final int MAX_LINE_LENGTH = 10000;
    private static final String KEY_VALUE_SEPARATOR = "=";
    
    public static class KeyValueRecord implements Comparable<KeyValueRecord> {
        private final String key;
        private final String value;
        
        public KeyValueRecord(String key, String value) {
            if (key == null || value == null) {
                throw new IllegalArgumentException("Key and value cannot be null");
            }
            this.key = key;
            this.value = value;
        }
        
        public String getKey() {
            return key;
        }
        
        public String getValue() {
            return value;
        }
        
        @Override
        public int compareTo(KeyValueRecord other) {
            return this.key.compareTo(other.key);
        }
        
        @Override
        public String toString() {
            return key + KEY_VALUE_SEPARATOR + value;
        }
    }
    
    public static List<KeyValueRecord> readAndSortRecords(String baseDir, String filename) throws IOException {
        if (baseDir == null || filename == null) {
            throw new IllegalArgumentException("Base directory and filename cannot be null");
        }
        
        // Normalize and validate paths
        Path basePath = Paths.get(baseDir).toAbsolutePath().normalize();
        Path filePath = basePath.resolve(filename).normalize();
        
        // Ensure file is within base directory
        if (!filePath.startsWith(basePath)) {
            throw new SecurityException("Path traversal detected: file must be within base directory");
        }
        
        // Check if file exists and is a regular file
        if (!Files.exists(filePath)) {
            throw new FileNotFoundException("File does not exist: " + filename);
        }
        
        if (!Files.isRegularFile(filePath, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Path is not a regular file: " + filename);
        }
        
        // Check file size
        long fileSize = Files.size(filePath);
        if (fileSize > MAX_FILE_SIZE) {
            throw new IOException("File size exceeds maximum allowed: " + fileSize);
        }
        
        List<KeyValueRecord> records = new ArrayList<>();
        
        try (BufferedReader reader = Files.newBufferedReader(filePath, StandardCharsets.UTF_8)) {
            String line;
            int lineNumber = 0;
            
            while ((line = reader.readLine()) != null) {
                lineNumber++;
                
                if (line.length() > MAX_LINE_LENGTH) {
                    throw new IOException("Line " + lineNumber + " exceeds maximum length");
                }
                
                // Skip empty lines and comments
                line = line.trim();
                if (line.isEmpty() || line.startsWith("#")) {
                    continue;
                }
                
                // Parse key-value pair
                int separatorIndex = line.indexOf(KEY_VALUE_SEPARATOR);
                if (separatorIndex <= 0 || separatorIndex >= line.length() - 1) {
                    throw new IOException("Invalid key-value format at line " + lineNumber + ": " + line);
                }
                
                String key = line.substring(0, separatorIndex).trim();
                String value = line.substring(separatorIndex + 1).trim();
                
                if (key.isEmpty()) {
                    throw new IOException("Empty key at line " + lineNumber);
                }
                
                records.add(new KeyValueRecord(key, value));
            }
        } catch (IOException e) {
            throw new IOException("Error reading file: " + e.getMessage(), e);
        }
        
        // Sort records by key
        Collections.sort(records);
        
        return records;
    }
    
    public static void main(String[] args) {
        // Test case setup
        String baseDir = System.getProperty("java.io.tmpdir");
        
        // Test case 1: Normal key-value pairs
        try {
            Path testFile1 = Paths.get(baseDir, "test1.txt");
            Files.write(testFile1, Arrays.asList("zebra=animal", "apple=fruit", "banana=fruit"), StandardCharsets.UTF_8);
            List<KeyValueRecord> result1 = readAndSortRecords(baseDir, "test1.txt");
            System.out.println("Test 1 - Normal sorting:");
            result1.forEach(System.out::println);
            Files.deleteIfExists(testFile1);
        } catch (Exception e) {
            System.err.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: With comments and empty lines
        try {
            Path testFile2 = Paths.get(baseDir, "test2.txt");
            Files.write(testFile2, Arrays.asList("# Comment line", "", "dog=pet", "cat=pet", ""), StandardCharsets.UTF_8);
            List<KeyValueRecord> result2 = readAndSortRecords(baseDir, "test2.txt");
            System.out.println("\\nTest 2 - With comments:");
            result2.forEach(System.out::println);
            Files.deleteIfExists(testFile2);
        } catch (Exception e) {
            System.err.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: Numeric keys
        try {
            Path testFile3 = Paths.get(baseDir, "test3.txt");
            Files.write(testFile3, Arrays.asList("3=three", "1=one", "2=two"), StandardCharsets.UTF_8);
            List<KeyValueRecord> result3 = readAndSortRecords(baseDir, "test3.txt");
            System.out.println("\\nTest 3 - Numeric keys:");
            result3.forEach(System.out::println);
            Files.deleteIfExists(testFile3);
        } catch (Exception e) {
            System.err.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test case 4: Values with spaces
        try {
            Path testFile4 = Paths.get(baseDir, "test4.txt");
            Files.write(testFile4, Arrays.asList("key1=value with spaces", "akey=another value"), StandardCharsets.UTF_8);
            List<KeyValueRecord> result4 = readAndSortRecords(baseDir, "test4.txt");
            System.out.println("\\nTest 4 - Values with spaces:");
            result4.forEach(System.out::println);
            Files.deleteIfExists(testFile4);
        } catch (Exception e) {
            System.err.println("Test 4 failed: " + e.getMessage());
        }
        
        // Test case 5: File not found
        try {
            List<KeyValueRecord> result5 = readAndSortRecords(baseDir, "nonexistent.txt");
            System.out.println("\\nTest 5 - File not found: FAILED (should throw exception)");
        } catch (Exception e) {
            System.out.println("\\nTest 5 - File not found: PASSED - " + e.getMessage());
        }
    }
}
