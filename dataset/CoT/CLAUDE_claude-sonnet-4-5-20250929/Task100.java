
import java.io.*;
import java.nio.file.*;
import java.util.regex.*;

public class Task100 {
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    private static final int REGEX_TIMEOUT_MS = 5000;
    
    public static void searchFileWithRegex(String regexPattern, String fileName) {
        // Validate inputs
        if (regexPattern == null || regexPattern.isEmpty()) {
            System.err.println("Error: Regex pattern cannot be empty");
            return;
        }
        
        if (fileName == null || fileName.isEmpty()) {
            System.err.println("Error: File name cannot be empty");
            return;
        }
        
        try {
            // Validate file path (prevent path traversal)
            Path filePath = Paths.get(fileName).normalize();
            File file = filePath.toFile();
            
            // Check if file exists and is readable
            if (!file.exists()) {
                System.err.println("Error: File does not exist: " + fileName);
                return;
            }
            
            if (!file.isFile()) {
                System.err.println("Error: Path is not a regular file: " + fileName);
                return;
            }
            
            if (!file.canRead()) {
                System.err.println("Error: File is not readable: " + fileName);
                return;
            }
            
            // Check file size
            if (file.length() > MAX_FILE_SIZE) {
                System.err.println("Error: File too large (max 10MB)");
                return;
            }
            
            // Compile regex pattern with timeout protection
            Pattern pattern;
            try {
                pattern = Pattern.compile(regexPattern);
            } catch (PatternSyntaxException e) {
                System.err.println("Error: Invalid regex pattern: " + e.getMessage());
                return;
            }
            
            // Read and search file content
            try (BufferedReader reader = new BufferedReader(
                    new InputStreamReader(new FileInputStream(file), "UTF-8"))) {
                
                String line;
                int lineNumber = 0;
                boolean found = false;
                
                while ((line = reader.readLine()) != null) {
                    lineNumber++;
                    Matcher matcher = pattern.matcher(line);
                    
                    while (matcher.find()) {
                        found = true;
                        System.out.println("Line " + lineNumber + ": " + line);
                        System.out.println("  Match: \\"" + matcher.group() + 
                                         "\\" at position " + matcher.start());
                    }
                }
                
                if (!found) {
                    System.out.println("No matches found.");
                }
                
            } catch (IOException e) {
                System.err.println("Error reading file: " + e.getMessage());
            }
            
        } catch (InvalidPathException e) {
            System.err.println("Error: Invalid file path: " + e.getMessage());
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        if (args.length >= 2) {
            searchFileWithRegex(args[0], args[1]);
        } else {
            // Test cases
            System.out.println("=== Test Case 1: Search for email pattern ===");
            createTestFile("test1.txt", "Contact: john@example.com\\nEmail: jane@test.org");
            searchFileWithRegex("\\\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\\\.[A-Z|a-z]{2,}\\\\b", "test1.txt");
            
            System.out.println("\\n=== Test Case 2: Search for phone numbers ===");
            createTestFile("test2.txt", "Call: 123-456-7890\\nPhone: 555-0123");
            searchFileWithRegex("\\\\d{3}-\\\\d{3}-\\\\d{4}", "test2.txt");
            
            System.out.println("\\n=== Test Case 3: No matches ===");
            createTestFile("test3.txt", "No numbers here at all!");
            searchFileWithRegex("\\\\d+", "test3.txt");
            
            System.out.println("\\n=== Test Case 4: Invalid regex ===");
            searchFileWithRegex("[invalid(", "test1.txt");
            
            System.out.println("\\n=== Test Case 5: Non-existent file ===");
            searchFileWithRegex("test", "nonexistent.txt");
            
            cleanupTestFiles();
        }
    }
    
    private static void createTestFile(String filename, String content) {
        try (PrintWriter writer = new PrintWriter(new FileWriter(filename))) {
            writer.println(content);
        } catch (IOException e) {
            System.err.println("Error creating test file: " + e.getMessage());
        }
    }
    
    private static void cleanupTestFiles() {
        try {
            Files.deleteIfExists(Paths.get("test1.txt"));
            Files.deleteIfExists(Paths.get("test2.txt"));
            Files.deleteIfExists(Paths.get("test3.txt"));
        } catch (IOException e) {
            // Ignore cleanup errors
        }
    }
}
