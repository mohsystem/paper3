
import java.io.*;
import java.nio.file.*;
import java.util.regex.*;

public class Task100 {
    public static String searchPatternInFile(String pattern, String filename) {
        StringBuilder result = new StringBuilder();
        
        try {
            // Validate input parameters
            if (pattern == null || pattern.isEmpty()) {
                return "Error: Pattern cannot be null or empty";
            }
            if (filename == null || filename.isEmpty()) {
                return "Error: Filename cannot be null or empty";
            }
            
            // Validate filename to prevent path traversal attacks
            Path filePath = Paths.get(filename).normalize();
            if (!Files.exists(filePath)) {
                return "Error: File does not exist";
            }
            if (!Files.isRegularFile(filePath)) {
                return "Error: Not a regular file";
            }
            if (!Files.isReadable(filePath)) {
                return "Error: File is not readable";
            }
            
            // Compile the regular expression with timeout protection
            Pattern regexPattern;
            try {
                regexPattern = Pattern.compile(pattern);
            } catch (PatternSyntaxException e) {
                return "Error: Invalid regex pattern - " + e.getMessage();
            }
            
            // Read file content with size limit (10MB)
            long fileSize = Files.size(filePath);
            if (fileSize > 10 * 1024 * 1024) {
                return "Error: File too large (max 10MB)";
            }
            
            String content = new String(Files.readAllBytes(filePath));
            Matcher matcher = regexPattern.matcher(content);
            
            int matchCount = 0;
            while (matcher.find() && matchCount < 1000) { // Limit matches
                result.append("Match found at position ")
                      .append(matcher.start())
                      .append(": ")
                      .append(matcher.group())
                      .append("\\n");
                matchCount++;
            }
            
            if (matchCount == 0) {
                result.append("No matches found\\n");
            } else {
                result.append("Total matches: ").append(matchCount).append("\\n");
            }
            
        } catch (IOException e) {
            return "Error reading file: " + e.getMessage();
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("=== Test Case 1: Search for word pattern ===");
        createTestFile("test1.txt", "Hello world! Hello Java programming.");
        System.out.println(searchPatternInFile("Hello", "test1.txt"));
        
        System.out.println("\\n=== Test Case 2: Search for email pattern ===");
        createTestFile("test2.txt", "Contact: user@example.com or admin@test.org");
        System.out.println(searchPatternInFile("\\\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\\\.[A-Z|a-z]{2,}\\\\b", "test2.txt"));
        
        System.out.println("\\n=== Test Case 3: Search for digit pattern ===");
        createTestFile("test3.txt", "The numbers are 123, 456, and 789.");
        System.out.println(searchPatternInFile("\\\\d+", "test3.txt"));
        
        System.out.println("\\n=== Test Case 4: Invalid pattern ===");
        System.out.println(searchPatternInFile("[invalid(", "test1.txt"));
        
        System.out.println("\\n=== Test Case 5: Non-existent file ===");
        System.out.println(searchPatternInFile("test", "nonexistent.txt"));
        
        // Cleanup
        deleteTestFile("test1.txt");
        deleteTestFile("test2.txt");
        deleteTestFile("test3.txt");
    }
    
    private static void createTestFile(String filename, String content) {
        try {
            Files.write(Paths.get(filename), content.getBytes());
        } catch (IOException e) {
            System.err.println("Error creating test file: " + e.getMessage());
        }
    }
    
    private static void deleteTestFile(String filename) {
        try {
            Files.deleteIfExists(Paths.get(filename));
        } catch (IOException e) {
            System.err.println("Error deleting test file: " + e.getMessage());
        }
    }
}
