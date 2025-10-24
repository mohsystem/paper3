
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.InvalidPathException;

public class Task36 {
    /* Maximum file size to prevent excessive memory usage: 10MB */
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024;
    
    /**
     * Security: Validates file path to prevent directory traversal attacks
     * @param filePath The file path to validate
     * @return true if valid, false otherwise
     */
    private static boolean validateFilePath(String filePath) {
        if (filePath == null || filePath.isEmpty()) {
            System.err.println("Error: File path is null or empty");
            return false;
        }
        
        /* Security: Reject paths with directory traversal patterns */
        if (filePath.contains("..")) {
            System.err.println("Error: Path traversal not allowed");
            return false;
        }
        
        /* Security: Reject absolute paths for security */
        if (filePath.startsWith("/") || filePath.matches("^[A-Za-z]:.*")) {
            System.err.println("Error: Absolute paths not allowed");
            return false;
        }
        
        return true;
    }
    
    /**
     * Security: Reads and prints file contents with validation and size limits
     * @param filename The name of the file to read
     * @return true on success, false on failure
     */
    public static boolean readAndPrintFile(String filename) {
        /* Security: Validate input path */
        if (!validateFilePath(filename)) {
            return false;
        }
        
        Path filePath = null;
        try {
            /* Security: Use Path API which handles normalization */
            filePath = Paths.get(filename).normalize();
            
            /* Security: Additional check after normalization */
            if (filePath.isAbsolute() || filePath.toString().contains("..")) {
                System.err.println("Error: Invalid path after normalization");
                return false;
            }
            
            /* Security: Check if file exists and is a regular file */
            if (!Files.exists(filePath)) {
                System.err.println("Error: File does not exist: " + filename);
                return false;
            }
            
            if (!Files.isRegularFile(filePath)) {
                System.err.println("Error: Not a regular file: " + filename);
                return false;
            }
            
            /* Security: Check file size before reading */
            long fileSize = Files.size(filePath);
            if (fileSize > MAX_FILE_SIZE) {
                System.err.println("Error: File exceeds maximum size limit");
                return false;
            }
            
            /* Security: Read file with proper encoding and error handling */
            byte[] fileBytes = Files.readAllBytes(filePath);
            String content = new String(fileBytes, StandardCharsets.UTF_8);
            
            /* Print file contents */
            System.out.print(content);
            
            return true;
            
        } catch (InvalidPathException e) {
            System.err.println("Error: Invalid file path: " + e.getMessage());
            return false;
        } catch (IOException e) {
            System.err.println("Error: Cannot read file: " + e.getMessage());
            return false;
        } catch (SecurityException e) {
            System.err.println("Error: Security exception: " + e.getMessage());
            return false;
        } catch (OutOfMemoryError e) {
            System.err.println("Error: File too large to read into memory");
            return false;
        }
    }
    
    public static void main(String[] args) {
        /* Test cases */
        String[] testFiles = {
            "test1.txt",
            "test2.txt",
            "test3.txt",
            "nonexistent.txt",
            "../etc/passwd"  /* Should be rejected */
        };
        
        if (args.length > 0) {
            /* User provided filename as argument */
            if (!readAndPrintFile(args[0])) {
                System.exit(1);
            }
        } else {
            /* Run test cases */
            System.out.println("Running test cases:");
            System.out.println("===================\\n");
            
            for (int i = 0; i < testFiles.length; i++) {
                System.out.println("Test " + (i + 1) + ": Reading file '" + testFiles[i] + "'");
                System.out.println("-------------------");
                
                if (readAndPrintFile(testFiles[i])) {
                    System.out.println();
                }
                
                System.out.println();
            }
        }
    }
}
