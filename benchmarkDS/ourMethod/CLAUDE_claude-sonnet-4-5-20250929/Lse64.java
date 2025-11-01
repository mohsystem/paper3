
import java.io.*;
import java.nio.file.*;
import java.sql.*;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse64 {
    private static final String UPLOAD_BASE_DIR = "/var/app/uploads";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final String DB_URL = "jdbc:mysql://localhost:3306/filedb?useSSL=true&requireSSL=true";
    private static final String DB_USER = "appuser";
    private static final String DB_PASS = System.getenv("DB_PASSWORD"); // Never hardcode passwords

    // Upload PDF: validates input, saves securely, stores path in database
    public static String uploadPdf(byte[] fileData, String originalFilename) throws Exception {
        // Input validation: check file data exists
        if (fileData == null || fileData.length == 0) {
            throw new IllegalArgumentException("File data is empty");
        }
        
        // Validate file size to prevent resource exhaustion
        if (fileData.length > MAX_FILE_SIZE) {
            throw new IllegalArgumentException("File size exceeds maximum allowed");
        }
        
        // Validate original filename: reject if null, empty, or contains path traversal
        if (originalFilename == null || originalFilename.isEmpty()) {
            throw new IllegalArgumentException("Invalid filename");
        }
        
        // Sanitize filename: remove any path separators and traversal attempts
        String sanitized = originalFilename.replaceAll("[^a-zA-Z0-9._-]", "_");
        if (sanitized.isEmpty()) {
            sanitized = "file.pdf";
        }
        
        // Validate PDF magic bytes (PDF files start with %PDF-)
        if (fileData.length < 5 || fileData[0] != 0x25 || fileData[1] != 0x50 || 
            fileData[2] != 0x44 || fileData[3] != 0x46 || fileData[4] != 0x2D) {
            throw new IllegalArgumentException("File is not a valid PDF");
        }
        
        // Generate cryptographically secure random filename to prevent collisions and guessing
        SecureRandom random = new SecureRandom();
        byte[] randomBytes = new byte[16];
        random.nextBytes(randomBytes);
        String randomName = Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes) + ".pdf";
        
        // Ensure upload directory exists with restricted permissions
        Path baseDir = Paths.get(UPLOAD_BASE_DIR).normalize();
        if (!Files.exists(baseDir)) {
            Files.createDirectories(baseDir);
            // Set directory permissions to 0700 (owner only)
            if (!System.getProperty("os.name").toLowerCase().contains("win")) {
                Files.setPosixFilePermissions(baseDir, 
                    java.nio.file.attribute.PosixFilePermissions.fromString("rwx------"));
            }
        }
        
        // Construct target path and validate it stays within base directory
        Path targetPath = baseDir.resolve(randomName).normalize();
        if (!targetPath.startsWith(baseDir)) {
            throw new SecurityException("Path traversal attempt detected");
        }
        
        // Write file atomically: write to temp, then atomic move
        Path tempFile = Files.createTempFile(baseDir, "upload_", ".tmp");
        try {
            // Set restrictive permissions on temp file (0600 - owner read/write only)
            if (!System.getProperty("os.name").toLowerCase().contains("win")) {
                Files.setPosixFilePermissions(tempFile, 
                    java.nio.file.attribute.PosixFilePermissions.fromString("rw-------"));
            }
            
            // Write data to temp file
            Files.write(tempFile, fileData);
            
            // Atomic move to final location
            Files.move(tempFile, targetPath, StandardCopyOption.ATOMIC_MOVE, 
                      StandardCopyOption.REPLACE_EXISTING);
        } catch (Exception e) {
            // Clean up temp file on failure
            try {
                Files.deleteIfExists(tempFile);
            } catch (IOException ignored) {}
            throw e;
        }
        
        // Store filepath in database using parameterized query to prevent SQL injection
        String relativePath = randomName; // Store only filename, not absolute path
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASS);
             PreparedStatement stmt = conn.prepareStatement(
                 "INSERT INTO uploaded_files (filepath, original_name, upload_time) VALUES (?, ?, NOW())")) {
            
            stmt.setString(1, relativePath);
            stmt.setString(2, sanitized); // Store sanitized original name for reference
            stmt.executeUpdate();
        }
        
        return relativePath;
    }
    
    // Download PDF: validates path, retrieves from secure location
    public static byte[] downloadPdf(String filepath) throws Exception {
        // Input validation: reject null or empty filepath
        if (filepath == null || filepath.isEmpty()) {
            throw new IllegalArgumentException("Invalid filepath");
        }
        
        // Sanitize and validate filepath from database
        String sanitized = filepath.replaceAll("[^a-zA-Z0-9._-]", "");
        if (sanitized.isEmpty() || !sanitized.endsWith(".pdf")) {
            throw new IllegalArgumentException("Invalid filepath format");
        }
        
        // Construct path and validate it stays within base directory
        Path baseDir = Paths.get(UPLOAD_BASE_DIR).normalize();
        Path targetPath = baseDir.resolve(sanitized).normalize();
        
        // Critical: ensure resolved path is within base directory (prevent path traversal)
        if (!targetPath.startsWith(baseDir)) {
            throw new SecurityException("Path traversal attempt detected");
        }
        
        // Open file using secure method with O_NOFOLLOW equivalent (don't follow symlinks)\n        // Validate it's a regular file and within size limits
        if (!Files.exists(targetPath, LinkOption.NOFOLLOW_LINKS)) {
            throw new FileNotFoundException("File not found");
        }
        
        if (!Files.isRegularFile(targetPath, LinkOption.NOFOLLOW_LINKS)) {
            throw new SecurityException("Not a regular file");
        }
        
        long size = Files.size(targetPath);
        if (size > MAX_FILE_SIZE || size < 5) {
            throw new IllegalArgumentException("File size out of bounds");
        }
        
        // Read file data
        byte[] data = Files.readAllBytes(targetPath);
        
        // Validate PDF magic bytes before returning
        if (data.length < 5 || data[0] != 0x25 || data[1] != 0x50 || 
            data[2] != 0x44 || data[3] != 0x46 || data[4] != 0x2D) {
            throw new SecurityException("File corruption detected");
        }
        
        return data;
    }
    
    public static void main(String[] args) {
        try {
            // Test case 1: Valid PDF upload
            byte[] validPdf = new byte[]{0x25, 0x50, 0x44, 0x46, 0x2D, 0x31, 0x2E, 0x34};
            String path1 = uploadPdf(validPdf, "document.pdf");
            System.out.println("Test 1 - Upload valid PDF: " + path1);
            
            // Test case 2: Download uploaded file
            byte[] downloaded = downloadPdf(path1);
            System.out.println("Test 2 - Download PDF: " + downloaded.length + " bytes");
            
            // Test case 3: Invalid PDF (missing magic bytes)
            try {
                uploadPdf(new byte[]{0x00, 0x00, 0x00}, "invalid.pdf");
                System.out.println("Test 3 - Should have rejected invalid PDF");
            } catch (IllegalArgumentException e) {
                System.out.println("Test 3 - Correctly rejected invalid PDF");
            }
            
            // Test case 4: Path traversal attempt in upload
            try {
                uploadPdf(validPdf, "../../../etc/passwd.pdf");
                System.out.println("Test 4 - Should have blocked path traversal");
            } catch (Exception e) {
                System.out.println("Test 4 - Correctly blocked path traversal");
            }
            
            // Test case 5: Path traversal attempt in download
            try {
                downloadPdf("../../../etc/passwd");
                System.out.println("Test 5 - Should have blocked path traversal");
            } catch (Exception e) {
                System.out.println("Test 5 - Correctly blocked path traversal");
            }
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
