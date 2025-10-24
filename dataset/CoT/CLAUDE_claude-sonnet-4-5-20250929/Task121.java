
import java.io.*;
import java.nio.file.*;
import java.security.MessageDigest;
import java.util.*;
import java.util.regex.Pattern;

public class Task121 {
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(
        Arrays.asList("txt", "pdf", "jpg", "jpeg", "png", "doc", "docx")
    );
    private static final String UPLOAD_DIR = "uploads";
    private static final Pattern SAFE_FILENAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_\\\\-\\\\.]+$");
    
    public static class UploadResult {
        public boolean success;
        public String message;
        public String savedPath;
        
        public UploadResult(boolean success, String message, String savedPath) {
            this.success = success;
            this.message = message;
            this.savedPath = savedPath;
        }
    }
    
    public static UploadResult uploadFile(String originalFilename, byte[] fileContent) {
        try {
            // Validate filename is not null or empty
            if (originalFilename == null || originalFilename.trim().isEmpty()) {
                return new UploadResult(false, "Filename cannot be empty", null);
            }
            
            // Validate file content is not null
            if (fileContent == null || fileContent.length == 0) {
                return new UploadResult(false, "File content cannot be empty", null);
            }
            
            // Check file size
            if (fileContent.length > MAX_FILE_SIZE) {
                return new UploadResult(false, "File size exceeds maximum limit of 10MB", null);
            }
            
            // Sanitize filename - remove path traversal attempts
            String sanitizedFilename = new File(originalFilename).getName();
            
            // Validate filename pattern
            if (!SAFE_FILENAME_PATTERN.matcher(sanitizedFilename).matches()) {
                return new UploadResult(false, "Invalid filename. Use only alphanumeric characters, hyphens, underscores, and dots", null);
            }
            
            // Validate file extension
            String extension = getFileExtension(sanitizedFilename).toLowerCase();
            if (!ALLOWED_EXTENSIONS.contains(extension)) {
                return new UploadResult(false, "File type not allowed. Allowed types: " + ALLOWED_EXTENSIONS, null);
            }
            
            // Create upload directory if it doesn't exist\n            File uploadDir = new File(UPLOAD_DIR);\n            if (!uploadDir.exists()) {\n                uploadDir.mkdirs();\n            }\n            \n            // Generate unique filename using timestamp and hash\n            String uniqueFilename = generateUniqueFilename(sanitizedFilename);\n            \n            // Construct safe file path\n            Path uploadPath = Paths.get(UPLOAD_DIR, uniqueFilename).normalize();\n            \n            // Verify the path is within upload directory (prevent path traversal)\n            if (!uploadPath.startsWith(Paths.get(UPLOAD_DIR).toAbsolutePath().normalize())) {\n                return new UploadResult(false, "Invalid upload path", null);\n            }\n            \n            // Write file securely\n            Files.write(uploadPath, fileContent, StandardOpenOption.CREATE_NEW);\n            \n            // Set file permissions (read/write for owner only)\n            File uploadedFile = uploadPath.toFile();\n            uploadedFile.setReadable(true, true);\n            uploadedFile.setWritable(true, true);\n            uploadedFile.setExecutable(false);\n            \n            return new UploadResult(true, "File uploaded successfully", uploadPath.toString());\n            \n        } catch (FileAlreadyExistsException e) {\n            return new UploadResult(false, "File already exists", null);\n        } catch (IOException e) {\n            return new UploadResult(false, "Error writing file: " + e.getMessage(), null);\n        } catch (Exception e) {\n            return new UploadResult(false, "Unexpected error: " + e.getMessage(), null);\n        }\n    }\n    \n    private static String getFileExtension(String filename) {\n        int lastDot = filename.lastIndexOf('.');\n        if (lastDot == -1 || lastDot == filename.length() - 1) {\n            return "";\n        }\n        return filename.substring(lastDot + 1);\n    }\n    \n    private static String generateUniqueFilename(String originalFilename) {\n        try {\n            String timestamp = String.valueOf(System.currentTimeMillis());\n            String baseName = originalFilename.substring(0, originalFilename.lastIndexOf('.'));\n            String extension = getFileExtension(originalFilename);\n            \n            MessageDigest md = MessageDigest.getInstance("SHA-256");\n            byte[] hash = md.digest((baseName + timestamp).getBytes());\n            String hashStr = bytesToHex(hash).substring(0, 8);\n            \n            return baseName + "_" + timestamp + "_" + hashStr + "." + extension;\n        } catch (Exception e) {\n            return originalFilename.replaceFirst("\\\\.", "_" + System.currentTimeMillis() + ".");\n        }\n    }\n    \n    private static String bytesToHex(byte[] bytes) {\n        StringBuilder sb = new StringBuilder();\n        for (byte b : bytes) {\n            sb.append(String.format("%02x", b));\n        }\n        return sb.toString();\n    }\n    \n    public static void main(String[] args) {\n        System.out.println("=== File Upload Program Test Cases ===\\n");\n        \n        // Test Case 1: Valid text file upload\n        System.out.println("Test 1: Valid text file upload");\n        byte[] content1 = "Hello, this is a test file content.".getBytes();\n        UploadResult result1 = uploadFile("test_document.txt", content1);\n        System.out.println("Success: " + result1.success);\n        System.out.println("Message: " + result1.message);\n        System.out.println("Path: " + result1.savedPath + "\\n");\n        \n        // Test Case 2: Invalid file extension\n        System.out.println("Test 2: Invalid file extension (.exe)");\n        byte[] content2 = "Malicious content".getBytes();\n        UploadResult result2 = uploadFile("malware.exe", content2);\n        System.out.println("Success: " + result2.success);\n        System.out.println("Message: " + result2.message + "\\n");\n        \n        // Test Case 3: Path traversal attempt\n        System.out.println("Test 3: Path traversal attempt");\n        byte[] content3 = "Attack content".getBytes();\n        UploadResult result3 = uploadFile("../../etc/passwd.txt", content3);\n        System.out.println("Success: " + result3.success);\n        System.out.println("Message: " + result3.message + "\\n");\n        \n        // Test Case 4: Empty filename\n        System.out.println("Test 4: Empty filename");\n        byte[] content4 = "Some content".getBytes();\n        UploadResult result4 = uploadFile("", content4);\n        System.out.println("Success: " + result4.success);\n        System.out.println("Message: " + result4.message + "\\n");\n        \n        // Test Case 5: File size exceeds limit\n        System.out.println("Test 5: File size exceeds limit");\n        byte[] content5 = new byte[(int)(MAX_FILE_SIZE + 1)];\n        Arrays.fill(content5, (byte)'A');
        UploadResult result5 = uploadFile("large_file.txt", content5);
        System.out.println("Success: " + result5.success);
        System.out.println("Message: " + result5.message + "\\n");
    }
}
