
import java.io.*;
import java.nio.file.*;
import java.security.MessageDigest;
import java.util.*;
import java.util.regex.Pattern;

public class Task50 {
    private static final String UPLOAD_DIR = "uploads";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(
        Arrays.asList("txt", "pdf", "jpg", "jpeg", "png", "doc", "docx")
    );
    
    public static String uploadFile(String filename, byte[] fileContent) {
        try {
            // Validate filename
            if (!isValidFilename(filename)) {
                return "Error: Invalid filename";
            }
            
            // Check file size
            if (fileContent.length > MAX_FILE_SIZE) {
                return "Error: File size exceeds maximum allowed size (10MB)";
            }
            
            // Check file extension
            String extension = getFileExtension(filename);
            if (!ALLOWED_EXTENSIONS.contains(extension.toLowerCase())) {
                return "Error: File type not allowed";
            }
            
            // Create upload directory if it doesn't exist\n            File uploadDir = new File(UPLOAD_DIR);\n            if (!uploadDir.exists()) {\n                uploadDir.mkdirs();\n            }\n            \n            // Sanitize filename and create unique name\n            String sanitizedName = sanitizeFilename(filename);\n            String uniqueFilename = generateUniqueFilename(sanitizedName);\n            String filepath = UPLOAD_DIR + File.separator + uniqueFilename;\n            \n            // Write file\n            Files.write(Paths.get(filepath), fileContent);\n            \n            // Calculate checksum\n            String checksum = calculateChecksum(fileContent);\n            \n            return "Success: File uploaded successfully. Filename: " + uniqueFilename + \n                   ", Size: " + fileContent.length + " bytes, Checksum: " + checksum;\n            \n        } catch (Exception e) {\n            return "Error: Failed to upload file - " + e.getMessage();\n        }\n    }\n    \n    private static boolean isValidFilename(String filename) {\n        if (filename == null || filename.trim().isEmpty()) {\n            return false;\n        }\n        // Check for path traversal attempts\n        if (filename.contains("..") || filename.contains("/") || filename.contains("\\\\")) {\n            return false;\n        }\n        // Check for valid characters\n        Pattern validPattern = Pattern.compile("^[a-zA-Z0-9._-]+$");\n        return validPattern.matcher(filename).matches();\n    }\n    \n    private static String getFileExtension(String filename) {\n        int lastDot = filename.lastIndexOf('.');\n        if (lastDot > 0 && lastDot < filename.length() - 1) {\n            return filename.substring(lastDot + 1);\n        }\n        return "";\n    }\n    \n    private static String sanitizeFilename(String filename) {\n        return filename.replaceAll("[^a-zA-Z0-9._-]", "_");\n    }\n    \n    private static String generateUniqueFilename(String filename) {\n        String timestamp = String.valueOf(System.currentTimeMillis());\n        String name = filename.substring(0, filename.lastIndexOf('.'));\n        String extension = getFileExtension(filename);\n        return name + "_" + timestamp + "." + extension;\n    }\n    \n    private static String calculateChecksum(byte[] data) throws Exception {\n        MessageDigest md = MessageDigest.getInstance("SHA-256");\n        byte[] hash = md.digest(data);\n        StringBuilder hexString = new StringBuilder();\n        for (byte b : hash) {\n            String hex = Integer.toHexString(0xff & b);\n            if (hex.length() == 1) hexString.append('0');
            hexString.append(hex);
        }
        return hexString.substring(0, 16);
    }
    
    public static void main(String[] args) {
        System.out.println("File Upload Server - Test Cases\\n");
        
        // Test Case 1: Valid text file
        String result1 = uploadFile("test1.txt", "Hello World!".getBytes());
        System.out.println("Test 1: " + result1 + "\\n");
        
        // Test Case 2: Valid PDF file (simulated)
        String result2 = uploadFile("document.pdf", "PDF content simulation".getBytes());
        System.out.println("Test 2: " + result2 + "\\n");
        
        // Test Case 3: Invalid filename with path traversal
        String result3 = uploadFile("../../../etc/passwd", "malicious content".getBytes());
        System.out.println("Test 3: " + result3 + "\\n");
        
        // Test Case 4: Disallowed file extension
        String result4 = uploadFile("script.exe", "executable content".getBytes());
        System.out.println("Test 4: " + result4 + "\\n");
        
        // Test Case 5: Valid image file
        String result5 = uploadFile("photo.jpg", "JPEG image data".getBytes());
        System.out.println("Test 5: " + result5 + "\\n");
    }
}
