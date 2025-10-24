
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import java.util.stream.Collectors;

public class Task93 {
    private static final int MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    private static final int MAX_LINE_LENGTH = 10000;
    private static final String KEY_VALUE_SEPARATOR = "=";
    
    public static List<Map.Entry<String, String>> readAndSortKeyValueFile(String filePath, Path baseDirectory) throws IOException {
        if (filePath == null || filePath.trim().isEmpty()) {
            throw new IllegalArgumentException("File path cannot be null or empty");
        }
        
        if (baseDirectory == null) {
            throw new IllegalArgumentException("Base directory cannot be null");
        }
        
        // Normalize and validate path
        Path requestedPath = baseDirectory.resolve(filePath).normalize();
        if (!requestedPath.startsWith(baseDirectory)) {
            throw new SecurityException("Path traversal attempt detected");
        }
        
        // Check file exists and is a regular file
        if (!Files.exists(requestedPath)) {
            throw new FileNotFoundException("File not found: " + filePath);
        }
        
        if (!Files.isRegularFile(requestedPath, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Not a regular file or symlink detected");
        }
        
        // Check file size
        long fileSize = Files.size(requestedPath);
        if (fileSize > MAX_FILE_SIZE) {
            throw new IOException("File size exceeds maximum allowed size");
        }
        
        Map<String, String> keyValueMap = new LinkedHashMap<>();
        
        try (BufferedReader reader = Files.newBufferedReader(requestedPath, StandardCharsets.UTF_8)) {
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
                    throw new IOException("Invalid key-value format at line " + lineNumber);
                }
                
                String key = line.substring(0, separatorIndex).trim();
                String value = line.substring(separatorIndex + 1).trim();
                
                if (key.isEmpty()) {
                    throw new IOException("Empty key at line " + lineNumber);
                }
                
                keyValueMap.put(key, value);
            }
        }
        
        // Sort by key
        return keyValueMap.entrySet().stream()
                .sorted(Map.Entry.comparingByKey())
                .collect(Collectors.toList());
    }
    
    public static void main(String[] args) {
        try {
            Path baseDir = Paths.get(System.getProperty("java.io.tmpdir"));
            
            // Test case 1: Normal key-value pairs
            Path testFile1 = baseDir.resolve("test1.txt");
            Files.write(testFile1, Arrays.asList("zebra=animal", "apple=fruit", "car=vehicle"), StandardCharsets.UTF_8);
            List<Map.Entry<String, String>> result1 = readAndSortKeyValueFile("test1.txt", baseDir);
            System.out.println("Test 1: " + result1);
            
            // Test case 2: With comments and empty lines
            Path testFile2 = baseDir.resolve("test2.txt");
            Files.write(testFile2, Arrays.asList("# Comment", "", "key2=value2", "key1=value1"), StandardCharsets.UTF_8);
            List<Map.Entry<String, String>> result2 = readAndSortKeyValueFile("test2.txt", baseDir);
            System.out.println("Test 2: " + result2);
            
            // Test case 3: Values with spaces
            Path testFile3 = baseDir.resolve("test3.txt");
            Files.write(testFile3, Arrays.asList("name=John Doe", "city=New York", "age=30"), StandardCharsets.UTF_8);
            List<Map.Entry<String, String>> result3 = readAndSortKeyValueFile("test3.txt", baseDir);
            System.out.println("Test 3: " + result3);
            
            // Test case 4: Single entry
            Path testFile4 = baseDir.resolve("test4.txt");
            Files.write(testFile4, Arrays.asList("single=value"), StandardCharsets.UTF_8);
            List<Map.Entry<String, String>> result4 = readAndSortKeyValueFile("test4.txt", baseDir);
            System.out.println("Test 4: " + result4);
            
            // Test case 5: Multiple entries with numeric keys
            Path testFile5 = baseDir.resolve("test5.txt");
            Files.write(testFile5, Arrays.asList("3=three", "1=one", "2=two"), StandardCharsets.UTF_8);
            List<Map.Entry<String, String>> result5 = readAndSortKeyValueFile("test5.txt", baseDir);
            System.out.println("Test 5: " + result5);
            
            // Cleanup
            Files.deleteIfExists(testFile1);
            Files.deleteIfExists(testFile2);
            Files.deleteIfExists(testFile3);
            Files.deleteIfExists(testFile4);
            Files.deleteIfExists(testFile5);
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
