
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.InvalidPathException;
import java.util.regex.Pattern;

public class Task65 {
    private static final Pattern SAFE_FILENAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_\\\\-\\\\.]+$");
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    
    public static String readFileSecurely(String filename) {
        if (filename == null || filename.trim().isEmpty()) {
            return "Error: Filename cannot be null or empty";
        }
        
        // Sanitize filename - prevent path traversal
        String sanitizedFilename = Paths.get(filename).getFileName().toString();
        
        // Validate filename format
        if (!SAFE_FILENAME_PATTERN.matcher(sanitizedFilename).matches()) {
            return "Error: Invalid filename format. Only alphanumeric, dots, hyphens, and underscores allowed";
        }
        
        try {
            Path filePath = Paths.get(sanitizedFilename).toRealPath();
            
            // Check if file exists and is readable
            if (!Files.exists(filePath)) {
                return "Error: File does not exist";
            }
            
            if (!Files.isRegularFile(filePath)) {
                return "Error: Not a regular file";
            }
            
            if (!Files.isReadable(filePath)) {
                return "Error: File is not readable";
            }
            
            // Check file size
            long fileSize = Files.size(filePath);
            if (fileSize > MAX_FILE_SIZE) {
                return "Error: File size exceeds maximum allowed size";
            }
            
            // Read file content
            byte[] fileBytes = Files.readAllBytes(filePath);
            return new String(fileBytes, "UTF-8");
            
        } catch (InvalidPathException e) {
            return "Error: Invalid file path - " + e.getMessage();
        } catch (IOException e) {
            return "Error: Unable to read file - " + e.getMessage();
        } catch (SecurityException e) {
            return "Error: Security violation - " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid file (you need to create test.txt)
        System.out.println("Test 1 - Valid file:");
        System.out.println(readFileSecurely("test.txt"));
        System.out.println();
        
        // Test case 2: Null filename
        System.out.println("Test 2 - Null filename:");
        System.out.println(readFileSecurely(null));
        System.out.println();
        
        // Test case 3: Empty filename
        System.out.println("Test 3 - Empty filename:");
        System.out.println(readFileSecurely(""));
        System.out.println();
        
        // Test case 4: Path traversal attempt
        System.out.println("Test 4 - Path traversal attempt:");
        System.out.println(readFileSecurely("../etc/passwd"));
        System.out.println();
        
        // Test case 5: Non-existent file
        System.out.println("Test 5 - Non-existent file:");
        System.out.println(readFileSecurely("nonexistent.txt"));
    }
}
