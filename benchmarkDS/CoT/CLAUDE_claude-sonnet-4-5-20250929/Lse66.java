
import java.io.*;
import java.nio.file.*;
import java.security.MessageDigest;
import java.util.UUID;
import java.util.regex.Pattern;

public class Lse66 {
    private static final String UPLOAD_DIR = "uploads";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final Pattern SAFE_FILENAME = Pattern.compile("^[a-zA-Z0-9_\\\\-\\\\.]+$");
    
    public static String uploadPdfFile(byte[] fileContent, String originalFilename) {
        try {
            // Validate file size
            if (fileContent == null || fileContent.length == 0) {
                return "Error: File is empty";
            }
            if (fileContent.length > MAX_FILE_SIZE) {
                return "Error: File size exceeds limit";
            }
            
            // Validate filename
            if (originalFilename == null || originalFilename.isEmpty()) {
                return "Error: Invalid filename";
            }
            
            // Validate PDF extension
            if (!originalFilename.toLowerCase().endsWith(".pdf")) {
                return "Error: Only PDF files are allowed";
            }
            
            // Validate PDF magic bytes
            if (!isPdfFile(fileContent)) {
                return "Error: File is not a valid PDF";
            }
            
            // Create uploads directory if not exists
            Path uploadPath = Paths.get(UPLOAD_DIR).toAbsolutePath().normalize();
            Files.createDirectories(uploadPath);
            
            // Generate safe filename
            String safeFilename = sanitizeFilename(originalFilename);
            String uniqueFilename = UUID.randomUUID().toString() + "_" + safeFilename;
            
            // Resolve target path and validate it's within upload directory
            Path targetPath = uploadPath.resolve(uniqueFilename).normalize();
            if (!targetPath.startsWith(uploadPath)) {
                return "Error: Invalid file path";
            }
            
            // Write file securely
            Files.write(targetPath, fileContent, StandardOpenOption.CREATE_NEW);
            
            // Set secure file permissions (read/write for owner only)
            if (!System.getProperty("os.name").toLowerCase().contains("win")) {
                Files.setPosixFilePermissions(targetPath, 
                    java.nio.file.attribute.PosixFilePermissions.fromString("rw-------"));
            }
            
            return "Success: File uploaded as " + uniqueFilename;
            
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    private static boolean isPdfFile(byte[] content) {
        if (content.length < 4) return false;
        return content[0] == 0x25 && content[1] == 0x50 && 
               content[2] == 0x44 && content[3] == 0x46; // %PDF
    }
    
    private static String sanitizeFilename(String filename) {
        String sanitized = filename.replaceAll("[^a-zA-Z0-9_\\\\-\\\\.]", "_");
        if (sanitized.length() > 100) {
            sanitized = sanitized.substring(0, 100);
        }
        return sanitized;
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid PDF
        byte[] validPdf = {0x25, 0x50, 0x44, 0x46, 0x2D, 0x31, 0x2E, 0x34};
        System.out.println("Test 1: " + uploadPdfFile(validPdf, "test.pdf"));
        
        // Test case 2: Invalid extension
        System.out.println("Test 2: " + uploadPdfFile(validPdf, "test.txt"));
        
        // Test case 3: Empty file
        System.out.println("Test 3: " + uploadPdfFile(new byte[0], "empty.pdf"));
        
        // Test case 4: Invalid PDF content
        byte[] invalidPdf = {0x00, 0x01, 0x02, 0x03};
        System.out.println("Test 4: " + uploadPdfFile(invalidPdf, "fake.pdf"));
        
        // Test case 5: Malicious filename
        System.out.println("Test 5: " + uploadPdfFile(validPdf, "../../../etc/passwd.pdf"));
    }
}
