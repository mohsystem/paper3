
import java.io.*;
import java.nio.file.*;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse66 {
    private static final String UPLOAD_DIR = "uploads";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    private static final String ALLOWED_CONTENT_TYPE = "application/pdf";
    private static final byte[] PDF_MAGIC = {0x25, 0x50, 0x44, 0x46}; // %PDF
    
    // Secure file upload that validates PDF and prevents path traversal
    public static String uploadPdf(InputStream fileStream, String originalFilename, long fileSize) throws IOException {
        // Input validation: check file size before reading
        if (fileSize <= 0 || fileSize > MAX_FILE_SIZE) {
            throw new IllegalArgumentException("Invalid file size");
        }
        
        // Validate filename is not null and doesn't contain path separators\n        if (originalFilename == null || originalFilename.isEmpty() || \n            originalFilename.contains("..") || originalFilename.contains("/") || \n            originalFilename.contains("\\\\")) {\n            throw new IllegalArgumentException("Invalid filename");\n        }\n        \n        // Check file extension\n        if (!originalFilename.toLowerCase().endsWith(".pdf")) {\n            throw new IllegalArgumentException("Only PDF files are allowed");\n        }\n        \n        // Create uploads directory with restricted permissions if it doesn't exist
        Path uploadPath = Paths.get(UPLOAD_DIR).toAbsolutePath().normalize();
        if (!Files.exists(uploadPath)) {
            Files.createDirectories(uploadPath);
            // Set restrictive permissions (owner read/write/execute only)
            try {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                perms.add(PosixFilePermission.OWNER_EXECUTE);
                Files.setPosixFilePermissions(uploadPath, perms);
            } catch (UnsupportedOperationException e) {
                // Windows doesn't support POSIX permissions, use basic file attributes\n            }\n        }\n        \n        // Generate secure random filename to prevent collisions and attacks\n        SecureRandom random = new SecureRandom();\n        byte[] randomBytes = new byte[16];\n        random.nextBytes(randomBytes);\n        String safeFilename = Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes) + ".pdf";\n        \n        // Construct target path and validate it's within upload directory
        Path targetPath = uploadPath.resolve(safeFilename).normalize();
        if (!targetPath.startsWith(uploadPath)) {
            throw new SecurityException("Path traversal attempt detected");
        }
        
        // Read file content with size limit enforcement
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        byte[] chunk = new byte[8192];
        int bytesRead;
        long totalRead = 0;
        
        while ((bytesRead = fileStream.read(chunk)) != -1) {
            totalRead += bytesRead;
            if (totalRead > MAX_FILE_SIZE) {
                throw new IOException("File size exceeds maximum allowed size");
            }
            buffer.write(chunk, 0, bytesRead);
        }
        
        byte[] fileContent = buffer.toByteArray();
        
        // Validate PDF magic bytes
        if (fileContent.length < 4) {
            throw new IllegalArgumentException("File too small to be a valid PDF");
        }
        
        for (int i = 0; i < PDF_MAGIC.length; i++) {
            if (fileContent[i] != PDF_MAGIC[i]) {
                throw new IllegalArgumentException("File is not a valid PDF");
            }
        }
        
        // Write to temporary file first, then atomically move to final location
        Path tempPath = Files.createTempFile(uploadPath, "temp_", ".pdf");
        
        try {
            // Set restrictive permissions on temp file
            try {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(tempPath, perms);
            } catch (UnsupportedOperationException e) {
                // Windows fallback
            }
            
            // Write content to temp file
            Files.write(tempPath, fileContent, StandardOpenOption.WRITE);
            
            // Atomic move to final location
            Files.move(tempPath, targetPath, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
            
            return safeFilename;
            
        } catch (Exception e) {
            // Clean up temp file on error
            Files.deleteIfExists(tempPath);
            throw e;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("PDF Upload Test Cases:");
        
        // Test case 1: Valid PDF file
        try {
            byte[] validPdf = new byte[100];
            validPdf[0] = 0x25; validPdf[1] = 0x50; validPdf[2] = 0x44; validPdf[3] = 0x46; // %PDF
            ByteArrayInputStream stream1 = new ByteArrayInputStream(validPdf);
            String result1 = uploadPdf(stream1, "document.pdf", validPdf.length);
            System.out.println("Test 1 PASSED: " + result1);
        } catch (Exception e) {
            System.out.println("Test 1 FAILED: " + e.getMessage());
        }
        
        // Test case 2: File too large
        try {
            uploadPdf(new ByteArrayInputStream(new byte[0]), "large.pdf", 20 * 1024 * 1024);
            System.out.println("Test 2 FAILED: Should reject large file");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 2 PASSED: Rejected large file");
        } catch (Exception e) {
            System.out.println("Test 2 FAILED: " + e.getMessage());
        }
        
        // Test case 3: Path traversal attempt
        try {
            byte[] validPdf = new byte[100];
            validPdf[0] = 0x25; validPdf[1] = 0x50; validPdf[2] = 0x44; validPdf[3] = 0x46;
            uploadPdf(new ByteArrayInputStream(validPdf), "../../../etc/passwd.pdf", validPdf.length);
            System.out.println("Test 3 FAILED: Should reject path traversal");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 3 PASSED: Rejected path traversal");
        } catch (Exception e) {
            System.out.println("Test 3 FAILED: " + e.getMessage());
        }
        
        // Test case 4: Invalid file extension
        try {
            byte[] validPdf = new byte[100];
            validPdf[0] = 0x25; validPdf[1] = 0x50; validPdf[2] = 0x44; validPdf[3] = 0x46;
            uploadPdf(new ByteArrayInputStream(validPdf), "document.exe", validPdf.length);
            System.out.println("Test 4 FAILED: Should reject non-PDF extension");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 4 PASSED: Rejected non-PDF extension");
        } catch (Exception e) {
            System.out.println("Test 4 FAILED: " + e.getMessage());
        }
        
        // Test case 5: Invalid PDF magic bytes
        try {
            byte[] invalidPdf = new byte[100];
            invalidPdf[0] = 0x00; invalidPdf[1] = 0x00; invalidPdf[2] = 0x00; invalidPdf[3] = 0x00;
            uploadPdf(new ByteArrayInputStream(invalidPdf), "fake.pdf", invalidPdf.length);
            System.out.println("Test 5 FAILED: Should reject invalid PDF");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 5 PASSED: Rejected invalid PDF magic bytes");
        } catch (Exception e) {
            System.out.println("Test 5 FAILED: " + e.getMessage());
        }
    }
}
