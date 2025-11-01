
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.file.*;
import java.security.MessageDigest;
import java.util.*;

public class Lse71 {
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(
        Arrays.asList("txt", "pdf", "jpg", "jpeg", "png", "gif", "doc", "docx")
    );
    private static final String UPLOAD_DIR = "uploads";

    public static String handleFileUpload(String filename, byte[] fileContent) {
        try {
            // Validate file size
            if (fileContent.length > MAX_FILE_SIZE) {
                return "Error: File size exceeds limit";
            }

            // Sanitize and validate filename
            String sanitizedFilename = sanitizeFilename(filename);
            if (sanitizedFilename == null) {
                return "Error: Invalid filename";
            }

            // Validate file extension
            String extension = getFileExtension(sanitizedFilename);
            if (!ALLOWED_EXTENSIONS.contains(extension.toLowerCase())) {
                return "Error: File type not allowed";
            }

            // Create upload directory if not exists
            Path uploadPath = Paths.get(UPLOAD_DIR);
            if (!Files.exists(uploadPath)) {
                Files.createDirectories(uploadPath);
            }

            // Generate secure filename
            String secureFilename = generateSecureFilename(sanitizedFilename);
            Path filePath = uploadPath.resolve(secureFilename);

            // Prevent path traversal
            if (!filePath.normalize().startsWith(uploadPath.normalize())) {
                return "Error: Invalid file path";
            }

            // Write file
            Files.write(filePath, fileContent);
            return "Success: File uploaded as " + secureFilename;

        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }

    private static String sanitizeFilename(String filename) {
        if (filename == null || filename.trim().isEmpty()) {
            return null;
        }
        // Remove path separators and special characters
        String sanitized = filename.replaceAll("[^a-zA-Z0-9._-]", "_");
        // Remove any remaining path traversal attempts
        sanitized = sanitized.replaceAll("\\\\.\\\\.", "");
        return sanitized.length() > 0 ? sanitized : null;
    }

    private static String getFileExtension(String filename) {
        int lastDot = filename.lastIndexOf('.');
        return lastDot > 0 ? filename.substring(lastDot + 1) : "";
    }

    private static String generateSecureFilename(String originalFilename) {
        try {
            String timestamp = String.valueOf(System.currentTimeMillis());
            String extension = getFileExtension(originalFilename);
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hash = md.digest((originalFilename + timestamp).getBytes());
            String hashString = bytesToHex(hash).substring(0, 16);
            return hashString + "." + extension;
        } catch (Exception e) {
            return UUID.randomUUID().toString() + "." + getFileExtension(originalFilename);
        }
    }

    private static String bytesToHex(byte[] bytes) {
        StringBuilder result = new StringBuilder();
        for (byte b : bytes) {
            result.append(String.format("%02x", b));
        }
        return result.toString();
    }

    public static void main(String[] args) {
        System.out.println("=== File Upload Security Test Cases ===\\n");

        // Test Case 1: Valid text file
        byte[] content1 = "Hello, World!".getBytes();
        System.out.println("Test 1 - Valid txt file: " + handleFileUpload("document.txt", content1));

        // Test Case 2: Valid PDF file
        byte[] content2 = "%PDF-1.4".getBytes();
        System.out.println("Test 2 - Valid pdf file: " + handleFileUpload("report.pdf", content2));

        // Test Case 3: Invalid file extension
        byte[] content3 = "malicious code".getBytes();
        System.out.println("Test 3 - Invalid extension: " + handleFileUpload("script.exe", content3));

        // Test Case 4: Path traversal attempt
        byte[] content4 = "test".getBytes();
        System.out.println("Test 4 - Path traversal: " + handleFileUpload("../../etc/passwd", content4));

        // Test Case 5: Large file (simulated)
        byte[] content5 = new byte[(int)(MAX_FILE_SIZE + 1)];
        System.out.println("Test 5 - Oversized file: " + handleFileUpload("large.txt", content5));
    }
}
