
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import java.util.regex.*;

public class Task119 {
    private static final int MAX_FILE_SIZE = 10_000_000; // 10MB
    private static final int MAX_FIELD_LENGTH = 10_000;
    private static final int MAX_FIELDS = 1000;
    private static final Pattern CSV_PATTERN = Pattern.compile(",(?=(?:[^\\"]*\\"[^\\"]*\\")*[^\\"]*$)");
    
    public static class CSVParser {
        private final String baseDir;
        
        public CSVParser(String baseDir) throws IOException {
            Path base = Paths.get(baseDir).toRealPath();
            this.baseDir = base.toString();
        }
        
        public List<List<String>> parseCSVFromString(String csvContent) {
            if (csvContent == null) {
                throw new IllegalArgumentException("CSV content cannot be null");
            }
            
            if (csvContent.length() > MAX_FILE_SIZE) {
                throw new IllegalArgumentException("CSV content exceeds maximum size");
            }
            
            List<List<String>> result = new ArrayList<>();
            String[] lines = csvContent.split("\\r?\\n");
            
            for (String line : lines) {
                if (line.trim().isEmpty()) {
                    continue;
                }
                
                List<String> fields = parseLine(line);
                if (fields.size() > MAX_FIELDS) {
                    throw new IllegalArgumentException("Too many fields in line");
                }
                result.add(fields);
            }
            
            return result;
        }
        
        public List<List<String>> parseCSVFromFile(String filename) throws IOException {
            if (filename == null || filename.isEmpty()) {
                throw new IllegalArgumentException("Filename cannot be null or empty");
            }
            
            // Validate filename - only allow alphanumeric, dash, underscore, and .csv
            if (!filename.matches("^[a-zA-Z0-9_-]+\\\\.csv$")) {
                throw new IllegalArgumentException("Invalid filename format");
            }
            
            Path filePath = Paths.get(baseDir, filename).normalize();
            
            // Prevent path traversal
            if (!filePath.startsWith(baseDir)) {
                throw new SecurityException("Path traversal attempt detected");
            }
            
            // Check if file exists and is a regular file
            if (!Files.exists(filePath) || !Files.isRegularFile(filePath)) {
                throw new FileNotFoundException("File not found or not a regular file");
            }
            
            // Check file size
            long fileSize = Files.size(filePath);
            if (fileSize > MAX_FILE_SIZE) {
                throw new IOException("File size exceeds maximum allowed size");
            }
            
            // Read file content
            String content = new String(Files.readAllBytes(filePath), StandardCharsets.UTF_8);
            return parseCSVFromString(content);
        }
        
        private List<String> parseLine(String line) {
            if (line == null) {
                return new ArrayList<>();
            }
            
            List<String> fields = new ArrayList<>();
            StringBuilder currentField = new StringBuilder();
            boolean inQuotes = false;
            
            for (int i = 0; i < line.length(); i++) {
                char c = line.charAt(i);
                
                if (c == '"') {
                    if (inQuotes && i + 1 < line.length() && line.charAt(i + 1) == '"') {
                        currentField.append('"');
                        i++;
                    } else {
                        inQuotes = !inQuotes;
                    }
                } else if (c == ',' && !inQuotes) {
                    String field = currentField.toString();
                    if (field.length() > MAX_FIELD_LENGTH) {
                        throw new IllegalArgumentException("Field exceeds maximum length");
                    }
                    fields.add(field);
                    currentField = new StringBuilder();
                } else {
                    currentField.append(c);
                }
            }
            
            String field = currentField.toString();
            if (field.length() > MAX_FIELD_LENGTH) {
                throw new IllegalArgumentException("Field exceeds maximum length");
            }
            fields.add(field);
            
            return fields;
        }
    }
    
    public static void main(String[] args) {
        try {
            // Test case 1: Simple CSV parsing from string
            System.out.println("Test 1: Simple CSV");
            String csv1 = "name,age,city\\nAlice,30,New York\\nBob,25,Los Angeles";
            CSVParser parser = new CSVParser(System.getProperty("java.io.tmpdir"));
            List<List<String>> result1 = parser.parseCSVFromString(csv1);
            for (List<String> row : result1) {
                System.out.println(row);
            }
            
            // Test case 2: CSV with quoted fields
            System.out.println("\\nTest 2: CSV with quotes");
            String csv2 = "name,description\\n\\"John Doe\\",\\"He said, \\"\\"Hello\\"\\"\\"\\nJane,Simple text";
            List<List<String>> result2 = parser.parseCSVFromString(csv2);
            for (List<String> row : result2) {
                System.out.println(row);
            }
            
            // Test case 3: Empty fields
            System.out.println("\\nTest 3: Empty fields");
            String csv3 = "a,b,c\\n1,,3\\n,2,\\n,,";
            List<List<String>> result3 = parser.parseCSVFromString(csv3);
            for (List<String> row : result3) {
                System.out.println(row);
            }
            
            // Test case 4: Single column
            System.out.println("\\nTest 4: Single column");
            String csv4 = "header\\nvalue1\\nvalue2\\nvalue3";
            List<List<String>> result4 = parser.parseCSVFromString(csv4);
            for (List<String> row : result4) {
                System.out.println(row);
            }
            
            // Test case 5: Special characters
            System.out.println("\\nTest 5: Special characters");
            String csv5 = "id,text\\n1,\\"Line1\\nLine2\\"\\n2,Tab\\there";
            List<List<String>> result5 = parser.parseCSVFromString(csv5);
            for (List<String> row : result5) {
                System.out.println(row);
            }
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
