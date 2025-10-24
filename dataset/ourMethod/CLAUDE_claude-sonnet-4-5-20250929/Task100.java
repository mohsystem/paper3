
import java.io.BufferedReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;
import java.util.regex.Matcher;

public class Task100 {
    private static final int MAX_FILE_SIZE = 10_000_000; // 10MB limit
    private static final String BASE_DIR = System.getProperty("user.dir");
    
    public static String searchFileWithPattern(String patternStr, String fileName) {
        if (patternStr == null || patternStr.isEmpty()) {
            return "Error: Pattern cannot be null or empty";
        }
        if (fileName == null || fileName.isEmpty()) {
            return "Error: File name cannot be null or empty";
        }
        
        // Validate and sanitize file path
        Path basePath = Paths.get(BASE_DIR).normalize().toAbsolutePath();
        Path filePath;
        try {
            filePath = basePath.resolve(fileName).normalize().toAbsolutePath();
            
            // Ensure the resolved path is within base directory
            if (!filePath.startsWith(basePath)) {
                return "Error: Access denied - path outside allowed directory";
            }
            
            // Check if it's a regular file
            if (!Files.isRegularFile(filePath)) {
                return "Error: Not a regular file";
            }
            
            // Check file size
            long fileSize = Files.size(filePath);
            if (fileSize > MAX_FILE_SIZE) {
                return "Error: File too large (max 10MB)";
            }
            
        } catch (IOException e) {
            return "Error: Cannot access file - " + e.getMessage();
        } catch (Exception e) {
            return "Error: Invalid file path - " + e.getMessage();
        }
        
        // Compile pattern
        Pattern pattern;
        try {
            pattern = Pattern.compile(patternStr);
        } catch (PatternSyntaxException e) {
            return "Error: Invalid regex pattern - " + e.getMessage();
        }
        
        // Search file content
        StringBuilder result = new StringBuilder();
        int matchCount = 0;
        
        try (BufferedReader reader = Files.newBufferedReader(filePath, StandardCharsets.UTF_8)) {
            String line;
            int lineNumber = 0;
            
            while ((line = reader.readLine()) != null) {
                lineNumber++;
                Matcher matcher = pattern.matcher(line);
                
                while (matcher.find()) {
                    matchCount++;
                    result.append("Line ").append(lineNumber).append(": ").append(line).append("\\n");
                    break; // Only report line once even if multiple matches
                }
            }
        } catch (IOException e) {
            return "Error: Failed to read file - " + e.getMessage();
        }
        
        if (matchCount == 0) {
            return "No matches found";
        }
        
        return "Found " + matchCount + " match(es):\\n" + result.toString();
    }
    
    public static void main(String[] args) {
        if (args.length == 2) {
            String pattern = args[0];
            String fileName = args[1];
            System.out.println(searchFileWithPattern(pattern, fileName));
        } else {
            // Test cases
            System.out.println("=== Test Case 1: Simple pattern ===");
            System.out.println(searchFileWithPattern("test", "testfile1.txt"));
            
            System.out.println("\\n=== Test Case 2: Complex pattern ===");
            System.out.println(searchFileWithPattern("[0-9]{3}", "testfile2.txt"));
            
            System.out.println("\\n=== Test Case 3: Invalid pattern ===");
            System.out.println(searchFileWithPattern("[invalid", "testfile1.txt"));
            
            System.out.println("\\n=== Test Case 4: Non-existent file ===");
            System.out.println(searchFileWithPattern("test", "nonexistent.txt"));
            
            System.out.println("\\n=== Test Case 5: Path traversal attempt ===");
            System.out.println(searchFileWithPattern("test", "../../../etc/passwd"));
        }
    }
}
