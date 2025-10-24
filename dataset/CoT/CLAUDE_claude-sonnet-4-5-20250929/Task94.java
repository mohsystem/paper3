
import java.io.*;
import java.nio.file.*;
import java.util.*;

public class Task94 {
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    
    public static List<Map.Entry<String, String>> readAndSortKeyValueFile(String filename) {
        List<Map.Entry<String, String>> records = new ArrayList<>();
        
        try {
            // Validate file path
            Path path = Paths.get(filename).normalize();
            if (!Files.exists(path)) {
                System.err.println("File not found: " + filename);
                return records;
            }
            
            // Check file size
            if (Files.size(path) > MAX_FILE_SIZE) {
                System.err.println("File size exceeds maximum allowed size");
                return records;
            }
            
            // Read and parse file
            try (BufferedReader reader = Files.newBufferedReader(path)) {
                String line;
                int lineNumber = 0;
                while ((line = reader.readLine()) != null) {
                    lineNumber++;
                    line = line.trim();
                    
                    // Skip empty lines
                    if (line.isEmpty()) {
                        continue;
                    }
                    
                    // Parse key-value pair (format: key=value or key:value)
                    String[] parts = line.split("[=:]", 2);
                    if (parts.length == 2) {
                        String key = parts[0].trim();
                        String value = parts[1].trim();
                        records.add(new AbstractMap.SimpleEntry<>(key, value));
                    } else {
                        System.err.println("Invalid format at line " + lineNumber + ": " + line);
                    }
                }
            }
            
            // Sort by key
            records.sort(Map.Entry.comparingByKey());
            
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        } catch (InvalidPathException e) {
            System.err.println("Invalid file path: " + e.getMessage());
        }
        
        return records;
    }
    
    public static void main(String[] args) {
        // Test case setup
        String testFile = "test_keyvalue.txt";
        
        try {
            // Create test file with sample data
            PrintWriter writer = new PrintWriter(testFile);
            writer.println("name=John");
            writer.println("age=30");
            writer.println("city=NewYork");
            writer.println("country=USA");
            writer.println("email=john@example.com");
            writer.close();
            
            System.out.println("Test Case 1: Basic key-value pairs");
            List<Map.Entry<String, String>> result1 = readAndSortKeyValueFile(testFile);
            for (Map.Entry<String, String> entry : result1) {
                System.out.println(entry.getKey() + " = " + entry.getValue());
            }
            
            // Test case 2: Colon separator
            writer = new PrintWriter(testFile);
            writer.println("zebra:animal");
            writer.println("apple:fruit");
            writer.println("carrot:vegetable");
            writer.close();
            
            System.out.println("\\nTest Case 2: Colon separator");
            List<Map.Entry<String, String>> result2 = readAndSortKeyValueFile(testFile);
            for (Map.Entry<String, String> entry : result2) {
                System.out.println(entry.getKey() + " : " + entry.getValue());
            }
            
            // Test case 3: Mixed with empty lines
            writer = new PrintWriter(testFile);
            writer.println("dog=animal");
            writer.println("");
            writer.println("banana=fruit");
            writer.println("apple=fruit");
            writer.close();
            
            System.out.println("\\nTest Case 3: With empty lines");
            List<Map.Entry<String, String>> result3 = readAndSortKeyValueFile(testFile);
            for (Map.Entry<String, String> entry : result3) {
                System.out.println(entry.getKey() + " = " + entry.getValue());
            }
            
            // Test case 4: Non-existent file
            System.out.println("\\nTest Case 4: Non-existent file");
            List<Map.Entry<String, String>> result4 = readAndSortKeyValueFile("nonexistent.txt");
            System.out.println("Records found: " + result4.size());
            
            // Test case 5: Invalid format
            writer = new PrintWriter(testFile);
            writer.println("validkey=validvalue");
            writer.println("invalidline");
            writer.println("anotherkey=anothervalue");
            writer.close();
            
            System.out.println("\\nTest Case 5: Invalid format handling");
            List<Map.Entry<String, String>> result5 = readAndSortKeyValueFile(testFile);
            for (Map.Entry<String, String> entry : result5) {
                System.out.println(entry.getKey() + " = " + entry.getValue());
            }
            
            // Cleanup
            new File(testFile).delete();
            
        } catch (IOException e) {
            System.err.println("Error in test setup: " + e.getMessage());
        }
    }
}
