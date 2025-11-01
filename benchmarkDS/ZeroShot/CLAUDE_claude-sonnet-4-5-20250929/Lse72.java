
import java.io.*;
import java.nio.file.*;
import java.security.MessageDigest;
import java.util.*;
import java.util.regex.Pattern;

public class Lse72 {
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(
        Arrays.asList("txt", "pdf", "jpg", "jpeg", "png", "doc", "docx")
    );
    private static final String UPLOAD_DIR = "uploads";
    
    public static String uploadFile(byte[] fileContent, String filename) {
        try {
            // Validate file size
            if (fileContent.length > MAX_FILE_SIZE) {
                return "Error: File size exceeds maximum allowed size";
            }
            
            // Sanitize filename
            String sanitizedFilename = sanitizeFilename(filename);
            if (sanitizedFilename == null) {
                return "Error: Invalid filename";
            }
            
            // Validate file extension
            if (!isAllowedExtension(sanitizedFilename)) {
                return "Error: File type not allowed";
            }
            
            // Create uploads directory if it doesn't exist\n            File uploadDir = new File(UPLOAD_DIR);\n            if (!uploadDir.exists()) {\n                uploadDir.mkdir();\n            }\n            \n            // Generate unique filename to prevent overwriting\n            String uniqueFilename = generateUniqueFilename(sanitizedFilename);\n            Path filePath = Paths.get(UPLOAD_DIR, uniqueFilename);\n            \n            // Save file\n            Files.write(filePath, fileContent);\n            \n            return "File uploaded successfully: " + uniqueFilename;\n        } catch (Exception e) {\n            return "Error: " + e.getMessage();\n        }\n    }\n    \n    private static String sanitizeFilename(String filename) {\n        if (filename == null || filename.isEmpty()) {\n            return null;\n        }\n        \n        // Remove path traversal attempts\n        filename = filename.replaceAll("\\\\.\\\\.", "");\n        filename = filename.replaceAll("[/\\\\\\\\]", "");\n        \n        // Allow only alphanumeric, dots, dashes, and underscores\n        if (!Pattern.matches("^[a-zA-Z0-9._-]+$", filename)) {\n            return null;\n        }\n        \n        return filename;\n    }\n    \n    private static boolean isAllowedExtension(String filename) {\n        int lastDot = filename.lastIndexOf('.');\n        if (lastDot == -1) {\n            return false;\n        }\n        String extension = filename.substring(lastDot + 1).toLowerCase();\n        return ALLOWED_EXTENSIONS.contains(extension);\n    }\n    \n    private static String generateUniqueFilename(String filename) {\n        String timestamp = String.valueOf(System.currentTimeMillis());\n        int lastDot = filename.lastIndexOf('.');
        if (lastDot == -1) {
            return timestamp + "_" + filename;
        }
        String name = filename.substring(0, lastDot);
        String extension = filename.substring(lastDot);
        return name + "_" + timestamp + extension;
    }
    
    public static void main(String[] args) {
        // Test Case 1: Valid text file
        String testContent1 = "This is a test file content";
        System.out.println(uploadFile(testContent1.getBytes(), "test1.txt"));
        
        // Test Case 2: Valid PDF file
        byte[] testContent2 = new byte[1024];
        System.out.println(uploadFile(testContent2, "document.pdf"));
        
        // Test Case 3: Invalid extension
        System.out.println(uploadFile(testContent1.getBytes(), "malicious.exe"));
        
        // Test Case 4: Path traversal attempt
        System.out.println(uploadFile(testContent1.getBytes(), "../../../etc/passwd"));
        
        // Test Case 5: Valid image file
        byte[] testContent5 = new byte[2048];
        System.out.println(uploadFile(testContent5, "photo.jpg"));
    }
}
