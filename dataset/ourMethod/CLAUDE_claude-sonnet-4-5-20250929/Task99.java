
import java.io.BufferedReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task99 {
    private static final Pattern LINE_PATTERN = Pattern.compile("^\\\\d+.*\\\\.$");
    private static final int MAX_LINE_LENGTH = 10000;
    private static final int MAX_LINES = 100000;
    
    public static List<String> matchLines(String baseDir, String fileName) {
        if (baseDir == null || fileName == null || baseDir.isEmpty() || fileName.isEmpty()) {
            throw new IllegalArgumentException("Base directory and file name must not be null or empty");
        }
        
        List<String> matchedLines = new ArrayList<>();
        
        try {
            Path basePath = Paths.get(baseDir).toRealPath();
            Path filePath = basePath.resolve(fileName).normalize();
            
            if (!filePath.startsWith(basePath)) {
                throw new SecurityException("Path traversal attempt detected");
            }
            
            if (!Files.isRegularFile(filePath)) {
                throw new IllegalArgumentException("Not a regular file");
            }
            
            if (Files.isSymbolicLink(filePath)) {
                throw new SecurityException("Symbolic links are not allowed");
            }
            
            int lineCount = 0;
            try (BufferedReader reader = Files.newBufferedReader(filePath, StandardCharsets.UTF_8)) {
                String line;
                while ((line = reader.readLine()) != null) {
                    if (++lineCount > MAX_LINES) {
                        throw new IllegalStateException("File exceeds maximum line limit");
                    }
                    
                    if (line.length() > MAX_LINE_LENGTH) {
                        continue;
                    }
                    
                    Matcher matcher = LINE_PATTERN.matcher(line);
                    if (matcher.matches()) {
                        matchedLines.add(line);
                    }
                }
            }
        } catch (IOException e) {
            throw new RuntimeException("Error reading file: " + e.getMessage());
        }
        
        return matchedLines;
    }
    
    public static void main(String[] args) {
        try {
            Path testDir = Files.createTempDirectory("test");
            String baseDir = testDir.toString();
            
            // Test case 1: Valid lines starting with numbers and ending with dot
            Path testFile1 = testDir.resolve("test1.txt");
            Files.write(testFile1, "123 This is a test.\\n456 Another line.\\nNo number here.\\n789 Valid.".getBytes(StandardCharsets.UTF_8));
            List<String> result1 = matchLines(baseDir, "test1.txt");
            System.out.println("Test 1: " + result1);
            
            // Test case 2: Mixed content
            Path testFile2 = testDir.resolve("test2.txt");
            Files.write(testFile2, "1 Start.\\n2No space but valid.\\nNot valid\\n3 End.".getBytes(StandardCharsets.UTF_8));
            List<String> result2 = matchLines(baseDir, "test2.txt");
            System.out.println("Test 2: " + result2);
            
            // Test case 3: Empty file
            Path testFile3 = testDir.resolve("test3.txt");
            Files.write(testFile3, "".getBytes(StandardCharsets.UTF_8));
            List<String> result3 = matchLines(baseDir, "test3.txt");
            System.out.println("Test 3: " + result3);
            
            // Test case 4: No matching lines
            Path testFile4 = testDir.resolve("test4.txt");
            Files.write(testFile4, "No match here\\nAnother no match\\n".getBytes(StandardCharsets.UTF_8));
            List<String> result4 = matchLines(baseDir, "test4.txt");
            System.out.println("Test 4: " + result4);
            
            // Test case 5: All lines match
            Path testFile5 = testDir.resolve("test5.txt");
            Files.write(testFile5, "0 Zero.\\n99 Ninety nine.\\n42 Answer.".getBytes(StandardCharsets.UTF_8));
            List<String> result5 = matchLines(baseDir, "test5.txt");
            System.out.println("Test 5: " + result5);
            
            Files.walk(testDir)
                .sorted((a, b) -> b.compareTo(a))
                .forEach(p -> {
                    try {
                        Files.deleteIfExists(p);
                    } catch (IOException e) {
                        // Ignore cleanup errors
                    }
                });
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
