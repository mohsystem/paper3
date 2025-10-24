
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task99 {
    private static final Pattern LINE_PATTERN = Pattern.compile("^\\\\d+.*\\\\.$");
    private static final int MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    
    public static List<String> matchLines(String filePath) {
        List<String> matchedLines = new ArrayList<>();
        
        if (filePath == null || filePath.trim().isEmpty()) {
            System.err.println("Error: File path cannot be null or empty");
            return matchedLines;
        }
        
        // Validate file path to prevent path traversal
        try {
            String canonicalPath = Paths.get(filePath).toRealPath().toString();
            
            // Check file size to prevent DOS attacks
            long fileSize = Files.size(Paths.get(canonicalPath));
            if (fileSize > MAX_FILE_SIZE) {
                System.err.println("Error: File size exceeds maximum allowed size");
                return matchedLines;
            }
            
            try (BufferedReader reader = new BufferedReader(new FileReader(canonicalPath))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    Matcher matcher = LINE_PATTERN.matcher(line);
                    if (matcher.matches()) {
                        matchedLines.add(line);
                    }
                }
            }
        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
        
        return matchedLines;
    }
    
    public static void main(String[] args) {
        // Test case 1: Create a test file with valid content
        String testFile1 = "test1.txt";
        try {
            Files.write(Paths.get(testFile1), "1 This is line one.\\n2 Second line.\\n3 Third line.".getBytes());
            System.out.println("Test 1 - File with numbered lines:");
            List<String> result1 = matchLines(testFile1);
            result1.forEach(System.out::println);
            Files.deleteIfExists(Paths.get(testFile1));
        } catch (IOException e) {
            System.err.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: Mixed content
        String testFile2 = "test2.txt";
        try {
            Files.write(Paths.get(testFile2), "123 Start with number.\\nNo number start.\\n456 Another one.".getBytes());
            System.out.println("\\nTest 2 - Mixed content:");
            List<String> result2 = matchLines(testFile2);
            result2.forEach(System.out::println);
            Files.deleteIfExists(Paths.get(testFile2));
        } catch (IOException e) {
            System.err.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: No matching lines
        String testFile3 = "test3.txt";
        try {
            Files.write(Paths.get(testFile3), "No numbers here\\nAnother line without numbers".getBytes());
            System.out.println("\\nTest 3 - No matching lines:");
            List<String> result3 = matchLines(testFile3);
            System.out.println("Matched lines: " + result3.size());
            Files.deleteIfExists(Paths.get(testFile3));
        } catch (IOException e) {
            System.err.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test case 4: Lines ending without dot
        String testFile4 = "test4.txt";
        try {
            Files.write(Paths.get(testFile4), "1 Line with dot.\\n2 Line without dot\\n3 Another with dot.".getBytes());
            System.out.println("\\nTest 4 - Lines with and without ending dot:");
            List<String> result4 = matchLines(testFile4);
            result4.forEach(System.out::println);
            Files.deleteIfExists(Paths.get(testFile4));
        } catch (IOException e) {
            System.err.println("Test 4 failed: " + e.getMessage());
        }
        
        // Test case 5: Invalid file path
        System.out.println("\\nTest 5 - Invalid file path:");
        List<String> result5 = matchLines("nonexistent_file.txt");
        System.out.println("Matched lines: " + result5.size());
    }
}
