
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.file.*;
import java.security.SecureRandom;
import java.util.*;

public class Lse70 {
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(
        Arrays.asList("txt", "pdf", "jpg", "jpeg", "png", "gif")
    );
    private static final String UPLOAD_DIR = "uploads";
    
    public static String upload_file(byte[] fileData, String originalFilename) {
        try {
            // Create uploads directory if it doesn't exist\n            Path uploadPath = Paths.get(UPLOAD_DIR);\n            if (!Files.exists(uploadPath)) {\n                Files.createDirectories(uploadPath);\n            }\n            \n            // Validate file size\n            if (fileData.length > MAX_FILE_SIZE) {\n                return "Error: File size exceeds limit";\n            }\n            \n            // Sanitize and validate filename\n            String sanitizedName = sanitizeFilename(originalFilename);\n            if (sanitizedName == null || sanitizedName.isEmpty()) {\n                return "Error: Invalid filename";\n            }\n            \n            // Validate file extension\n            String extension = getFileExtension(sanitizedName);\n            if (!ALLOWED_EXTENSIONS.contains(extension.toLowerCase())) {\n                return "Error: File type not allowed";\n            }\n            \n            // Generate unique filename\n            String uniqueFilename = generateUniqueFilename(sanitizedName);\n            Path filePath = uploadPath.resolve(uniqueFilename);\n            \n            // Prevent path traversal\n            if (!filePath.normalize().startsWith(uploadPath.normalize())) {\n                return "Error: Invalid file path";\n            }\n            \n            // Save file\n            Files.write(filePath, fileData);\n            return "Success: File uploaded as " + uniqueFilename;\n            \n        } catch (Exception e) {\n            return "Error: " + e.getMessage();\n        }\n    }\n    \n    private static String sanitizeFilename(String filename) {\n        if (filename == null) return null;\n        return filename.replaceAll("[^a-zA-Z0-9._-]", "_");\n    }\n    \n    private static String getFileExtension(String filename) {\n        int lastDot = filename.lastIndexOf('.');\n        return (lastDot > 0) ? filename.substring(lastDot + 1) : "";\n    }\n    \n    private static String generateUniqueFilename(String originalName) {\n        String extension = getFileExtension(originalName);\n        String nameWithoutExt = originalName.substring(0, originalName.lastIndexOf('.'));
        SecureRandom random = new SecureRandom();
        return nameWithoutExt + "_" + System.currentTimeMillis() + 
               "_" + random.nextInt(10000) + "." + extension;
    }
    
    public static void main(String[] args) throws Exception {
        System.out.println("=== File Upload Test Cases ===\\n");
        
        // Test case 1: Valid text file
        byte[] testFile1 = "Hello, World!".getBytes();
        System.out.println("Test 1 - Valid text file: " + 
            upload_file(testFile1, "test.txt"));
        
        // Test case 2: Valid image file
        byte[] testFile2 = new byte[1024];
        System.out.println("Test 2 - Valid image file: " + 
            upload_file(testFile2, "image.jpg"));
        
        // Test case 3: Invalid extension
        byte[] testFile3 = "script content".getBytes();
        System.out.println("Test 3 - Invalid extension: " + 
            upload_file(testFile3, "malicious.exe"));
        
        // Test case 4: Path traversal attempt
        byte[] testFile4 = "data".getBytes();
        System.out.println("Test 4 - Path traversal attempt: " + 
            upload_file(testFile4, "../../../etc/passwd.txt"));
        
        // Test case 5: Empty filename
        byte[] testFile5 = "content".getBytes();
        System.out.println("Test 5 - Empty filename: " + 
            upload_file(testFile5, ""));
    }
}
