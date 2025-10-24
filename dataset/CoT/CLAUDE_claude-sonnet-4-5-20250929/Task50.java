
import java.io.*;
import java.nio.file.*;
import java.security.MessageDigest;
import java.util.*;
import java.util.regex.Pattern;

public class Task50 {
    private static final String UPLOAD_DIR = "uploads";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(
        Arrays.asList("txt", "pdf", "jpg", "jpeg", "png", "gif", "doc", "docx")
    );
    private static final Pattern SAFE_FILENAME_PATTERN = Pattern.compile("^[a-zA-Z0-9._-]+$");

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

    public static UploadResult uploadFile(String filename, byte[] fileContent) {
        try {
            // Validate filename
            if (filename == null || filename.trim().isEmpty()) {
                return new UploadResult(false, "Invalid filename", null);
            }

            // Sanitize filename - remove path components
            String sanitizedFilename = Paths.get(filename).getFileName().toString();
            
            // Additional filename validation
            if (!SAFE_FILENAME_PATTERN.matcher(sanitizedFilename).matches()) {
                sanitizedFilename = sanitizedFilename.replaceAll("[^a-zA-Z0-9._-]", "_");
            }

            // Validate file extension
            String extension = getFileExtension(sanitizedFilename);
            if (!ALLOWED_EXTENSIONS.contains(extension.toLowerCase())) {
                return new UploadResult(false, "File type not allowed: " + extension, null);
            }

            // Validate file size
            if (fileContent == null || fileContent.length == 0) {
                return new UploadResult(false, "Empty file content", null);
            }
            if (fileContent.length > MAX_FILE_SIZE) {
                return new UploadResult(false, "File size exceeds maximum limit", null);
            }

            // Create upload directory if it doesn't exist\n            Path uploadPath = Paths.get(UPLOAD_DIR);\n            if (!Files.exists(uploadPath)) {\n                Files.createDirectories(uploadPath);\n            }\n\n            // Generate unique filename to prevent overwriting\n            String uniqueFilename = generateUniqueFilename(sanitizedFilename);\n            Path targetPath = uploadPath.resolve(uniqueFilename);\n\n            // Ensure the resolved path is within the upload directory (prevent path traversal)\n            if (!targetPath.normalize().startsWith(uploadPath.normalize())) {\n                return new UploadResult(false, "Invalid file path detected", null);\n            }\n\n            // Write file securely\n            Files.write(targetPath, fileContent, \n                StandardOpenOption.CREATE_NEW, \n                StandardOpenOption.WRITE);\n\n            // Set file permissions (read/write for owner only)\n            if (!System.getProperty("os.name").toLowerCase().contains("win")) {\n                Files.setPosixFilePermissions(targetPath, \n                    PosixFilePermissions.fromString("rw-------"));\n            }\n\n            return new UploadResult(true, \n                "File uploaded successfully: " + uniqueFilename, \n                targetPath.toString());\n\n        } catch (IOException e) {\n            return new UploadResult(false, \n                "Upload failed: " + e.getMessage(), null);\n        }\n    }\n\n    private static String getFileExtension(String filename) {\n        int lastDot = filename.lastIndexOf('.');\n        if (lastDot > 0 && lastDot < filename.length() - 1) {\n            return filename.substring(lastDot + 1);\n        }\n        return "";\n    }\n\n    private static String generateUniqueFilename(String originalFilename) {\n        String timestamp = String.valueOf(System.currentTimeMillis());\n        String nameWithoutExt = originalFilename.substring(0, \n            originalFilename.lastIndexOf('.') > 0 ? \n            originalFilename.lastIndexOf('.') : originalFilename.length());\n        String extension = getFileExtension(originalFilename);\n        \n        return nameWithoutExt + "_" + timestamp + \n            (extension.isEmpty() ? "" : "." + extension);\n    }\n\n    public static void main(String[] args) {\n        System.out.println("=== Secure File Upload Server - Test Cases ===\\n");\n\n        // Test Case 1: Valid text file upload\n        String testContent1 = "This is a test file content.";\n        UploadResult result1 = uploadFile("test_document.txt", testContent1.getBytes());\n        System.out.println("Test 1 - Valid text file:");\n        System.out.println("Success: " + result1.success);\n        System.out.println("Message: " + result1.message);\n        System.out.println();\n\n        // Test Case 2: Valid PDF file upload\n        byte[] testContent2 = new byte[1024];\n        Arrays.fill(testContent2, (byte) 'A');
        UploadResult result2 = uploadFile("report.pdf", testContent2);
        System.out.println("Test 2 - Valid PDF file:");
        System.out.println("Success: " + result2.success);
        System.out.println("Message: " + result2.message);
        System.out.println();

        // Test Case 3: Invalid file extension
        UploadResult result3 = uploadFile("malicious.exe", "malware".getBytes());
        System.out.println("Test 3 - Invalid extension (.exe):");
        System.out.println("Success: " + result3.success);
        System.out.println("Message: " + result3.message);
        System.out.println();

        // Test Case 4: Path traversal attempt
        UploadResult result4 = uploadFile("../../etc/passwd.txt", "hack".getBytes());
        System.out.println("Test 4 - Path traversal attempt:");
        System.out.println("Success: " + result4.success);
        System.out.println("Message: " + result4.message);
        System.out.println();

        // Test Case 5: Empty file
        UploadResult result5 = uploadFile("empty.txt", new byte[0]);
        System.out.println("Test 5 - Empty file:");
        System.out.println("Success: " + result5.success);
        System.out.println("Message: " + result5.message);
        System.out.println();
    }
}
