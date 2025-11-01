
import java.io.*;
import java.nio.file.*;

public class Lse45 {
    public static String readFileFromSafeDirectory(String filename, String safeDirectory) {
        try {
            // Normalize and resolve the safe directory path
            Path safePath = Paths.get(safeDirectory).toRealPath();
            
            // Normalize and resolve the requested file path
            Path requestedPath = safePath.resolve(filename).normalize();
            
            // Check if the requested path is within the safe directory
            if (!requestedPath.startsWith(safePath)) {
                return "Error: Access denied - file is outside safe directory";
            }
            
            // Check if file exists
            if (!Files.exists(requestedPath)) {
                return "Error: File not found";
            }
            
            // Read and return file content
            return new String(Files.readAllBytes(requestedPath));
            
        } catch (IOException e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Create a test safe directory and files
        String safeDir = "safe_directory";
        try {
            Files.createDirectories(Paths.get(safeDir));
            Files.write(Paths.get(safeDir, "test1.txt"), "Content of test1".getBytes());
            Files.write(Paths.get(safeDir, "test2.txt"), "Content of test2".getBytes());
        } catch (IOException e) {
            System.out.println("Setup error: " + e.getMessage());
        }
        
        // Test case 1: Valid file in safe directory
        System.out.println("Test 1: " + readFileFromSafeDirectory("test1.txt", safeDir));
        
        // Test case 2: Another valid file
        System.out.println("Test 2: " + readFileFromSafeDirectory("test2.txt", safeDir));
        
        // Test case 3: Non-existent file
        System.out.println("Test 3: " + readFileFromSafeDirectory("nonexistent.txt", safeDir));
        
        // Test case 4: Path traversal attempt
        System.out.println("Test 4: " + readFileFromSafeDirectory("../test.txt", safeDir));
        
        // Test case 5: Another path traversal attempt
        System.out.println("Test 5: " + readFileFromSafeDirectory("../../etc/passwd", safeDir));
    }
}
