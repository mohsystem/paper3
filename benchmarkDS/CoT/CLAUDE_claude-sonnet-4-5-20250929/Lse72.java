
import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.regex.*;

public class Lse72 {
    private static final String UPLOAD_DIR = "uploads";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(
        Arrays.asList(".txt", ".pdf", ".jpg", ".png", ".doc", ".docx")
    );
    
    public static String uploadFile(byte[] fileContent, String filename) {
        try {
            // Validate filename
            if (filename == null || filename.trim().isEmpty()) {
                return "Error: Invalid filename";
            }
            
            // Sanitize filename - remove path traversal attempts
            String sanitizedFilename = sanitizeFilename(filename);
            if (sanitizedFilename.isEmpty()) {
                return "Error: Invalid filename after sanitization";
            }
            
            // Validate file extension
            if (!isAllowedExtension(sanitizedFilename)) {
                return "Error: File type not allowed";
            }
            
            // Validate file size
            if (fileContent.length > MAX_FILE_SIZE) {
                return "Error: File size exceeds limit";
            }
            
            // Create uploads directory if it doesn't exist\n            Path uploadPath = Paths.get(UPLOAD_DIR).toAbsolutePath();\n            if (!Files.exists(uploadPath)) {\n                Files.createDirectories(uploadPath);\n            }\n            \n            // Create secure file path\n            Path filePath = uploadPath.resolve(sanitizedFilename).normalize();\n            \n            // Verify the resolved path is still within uploads directory\n            if (!filePath.startsWith(uploadPath)) {\n                return "Error: Invalid file path";\n            }\n            \n            // Save file\n            Files.write(filePath, fileContent);\n            \n            return "Success: File uploaded to " + filePath.toString();\n        } catch (IOException e) {\n            return "Error: " + e.getMessage();\n        }\n    }\n    \n    private static String sanitizeFilename(String filename) {\n        // Remove path components\n        String name = Paths.get(filename).getFileName().toString();\n        \n        // Remove any character that isn't alphanumeric, dot, dash, or underscore
        name = name.replaceAll("[^a-zA-Z0-9._-]", "_");
        
        // Remove leading dots to prevent hidden files
        name = name.replaceAll("^\\\\.+", "");
        
        // Limit filename length
        if (name.length() > 255) {
            name = name.substring(0, 255);
        }
        
        return name;
    }
    
    private static boolean isAllowedExtension(String filename) {
        String lowerFilename = filename.toLowerCase();
        return ALLOWED_EXTENSIONS.stream().anyMatch(lowerFilename::endsWith);
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid file
        byte[] content1 = "Test content".getBytes();
        System.out.println("Test 1: " + uploadFile(content1, "document.txt"));
        
        // Test case 2: Path traversal attempt
        byte[] content2 = "Malicious content".getBytes();
        System.out.println("Test 2: " + uploadFile(content2, "../../../etc/passwd"));
        
        // Test case 3: Invalid extension
        byte[] content3 = "Script content".getBytes();
        System.out.println("Test 3: " + uploadFile(content3, "malicious.exe"));
        
        // Test case 4: Valid image file
        byte[] content4 = new byte[1024];
        System.out.println("Test 4: " + uploadFile(content4, "photo.jpg"));
        
        // Test case 5: Empty filename
        byte[] content5 = "Content".getBytes();
        System.out.println("Test 5: " + uploadFile(content5, ""));
    }
}
