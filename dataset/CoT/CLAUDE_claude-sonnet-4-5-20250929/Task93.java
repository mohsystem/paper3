
import java.io.*;
import java.nio.file.*;
import java.util.*;

public class Task93 {
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    
    public static class KeyValuePair implements Comparable<KeyValuePair> {
        private final String key;
        private final String value;
        
        public KeyValuePair(String key, String value) {
            this.key = key != null ? key : "";
            this.value = value != null ? value : "";
        }
        
        public String getKey() {
            return key;
        }
        
        public String getValue() {
            return value;
        }
        
        @Override
        public int compareTo(KeyValuePair other) {
            return this.key.compareTo(other.key);
        }
        
        @Override
        public String toString() {
            return key + "=" + value;
        }
    }
    
    public static List<KeyValuePair> readAndSortKeyValueFile(String filePath) {
        List<KeyValuePair> records = new ArrayList<>();
        
        if (filePath == null || filePath.trim().isEmpty()) {
            System.err.println("Invalid file path");
            return records;
        }
        
        Path path = Paths.get(filePath).normalize();
        
        try {
            if (Files.size(path) > MAX_FILE_SIZE) {
                System.err.println("File size exceeds maximum allowed size");
                return records;
            }
            
            try (BufferedReader reader = Files.newBufferedReader(path)) {
                String line;
                int lineNumber = 0;
                
                while ((line = reader.readLine()) != null) {
                    lineNumber++;
                    line = line.trim();
                    
                    if (line.isEmpty() || line.startsWith("#")) {
                        continue;
                    }
                    
                    String[] parts = line.split("=", 2);
                    if (parts.length == 2) {
                        String key = parts[0].trim();
                        String value = parts[1].trim();
                        records.add(new KeyValuePair(key, value));
                    } else {
                        System.err.println("Malformed line " + lineNumber + ": " + line);
                    }
                }
            }
            
            Collections.sort(records);
            
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        } catch (Exception e) {
            System.err.println("Unexpected error: " + e.getMessage());
        }
        
        return records;
    }
    
    public static void main(String[] args) {
        // Test case 1: Create and read a valid file
        try {
            String testFile1 = "test1.txt";
            Files.write(Paths.get(testFile1), Arrays.asList(
                "name=John",
                "age=30",
                "city=NewYork",
                "country=USA"
            ));
            System.out.println("Test 1 - Valid file:");
            List<KeyValuePair> result1 = readAndSortKeyValueFile(testFile1);
            result1.forEach(System.out::println);
            Files.deleteIfExists(Paths.get(testFile1));
            System.out.println();
        } catch (IOException e) {
            System.err.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: File with comments and empty lines
        try {
            String testFile2 = "test2.txt";
            Files.write(Paths.get(testFile2), Arrays.asList(
                "# This is a comment",
                "zebra=animal",
                "",
                "apple=fruit",
                "banana=fruit"
            ));
            System.out.println("Test 2 - File with comments:");
            List<KeyValuePair> result2 = readAndSortKeyValueFile(testFile2);
            result2.forEach(System.out::println);
            Files.deleteIfExists(Paths.get(testFile2));
            System.out.println();
        } catch (IOException e) {
            System.err.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: File with malformed lines
        try {
            String testFile3 = "test3.txt";
            Files.write(Paths.get(testFile3), Arrays.asList(
                "valid=line",
                "invalid line without equals",
                "another=valid"
            ));
            System.out.println("Test 3 - Malformed lines:");
            List<KeyValuePair> result3 = readAndSortKeyValueFile(testFile3);
            result3.forEach(System.out::println);
            Files.deleteIfExists(Paths.get(testFile3));
            System.out.println();
        } catch (IOException e) {
            System.err.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test case 4: Non-existent file
        System.out.println("Test 4 - Non-existent file:");
        List<KeyValuePair> result4 = readAndSortKeyValueFile("nonexistent.txt");
        System.out.println("Records found: " + result4.size());
        System.out.println();
        
        // Test case 5: Empty file path
        System.out.println("Test 5 - Empty file path:");
        List<KeyValuePair> result5 = readAndSortKeyValueFile("");
        System.out.println("Records found: " + result5.size());
    }
}
