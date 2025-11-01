
import java.io.*;
import java.nio.file.*;
import java.security.SecureRandom;
import java.util.*;

public class Lse72 {
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(
        Arrays.asList("txt", "pdf", "jpg", "jpeg", "png", "doc", "docx")
    );
    private static final String UPLOAD_DIR = "uploads";
    
    // Secure file upload handler
    public static String uploadFile(String originalFilename, InputStream fileContent) throws IOException {
        // Input validation: reject null or empty filename
        if (originalFilename == null || originalFilename.trim().isEmpty()) {
            throw new IllegalArgumentException("Invalid filename");
        }
        
        // Normalize and validate filename to prevent path traversal
        String sanitizedFilename = sanitizeFilename(originalFilename);
        if (sanitizedFilename == null || sanitizedFilename.isEmpty()) {
            throw new IllegalArgumentException("Invalid filename after sanitization");
        }
        
        // Validate file extension
        String extension = getFileExtension(sanitizedFilename);
        if (!ALLOWED_EXTENSIONS.contains(extension.toLowerCase())) {
            throw new IllegalArgumentException("File type not allowed");
        }
        
        // Create uploads directory with restrictive permissions if it doesn't exist\n        Path uploadDir = Paths.get(UPLOAD_DIR).toAbsolutePath().normalize();\n        if (!Files.exists(uploadDir)) {\n            Files.createDirectories(uploadDir);\n            // Set permissions to owner only (rwx------)\n            try {\n                Set<PosixFilePermission> perms = new HashSet<>();\n                perms.add(PosixFilePermission.OWNER_READ);\n                perms.add(PosixFilePermission.OWNER_WRITE);\n                perms.add(PosixFilePermission.OWNER_EXECUTE);\n                Files.setPosixFilePermissions(uploadDir, perms);\n            } catch (UnsupportedOperationException e) {\n                // Windows doesn't support POSIX permissions
            }
        }
        
        // Generate unique filename to prevent collisions and overwrites
        String uniqueFilename = generateUniqueFilename(sanitizedFilename);
        Path targetPath = uploadDir.resolve(uniqueFilename).normalize();
        
        // Validate that resolved path is within upload directory (prevent traversal)
        if (!targetPath.startsWith(uploadDir)) {
            throw new SecurityException("Path traversal attempt detected");
        }
        
        // Write to temporary file first, then atomic rename
        Path tempFile = Files.createTempFile(uploadDir, "upload_", ".tmp");
        
        try {
            // Set restrictive permissions on temp file
            try {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(tempFile, perms);
            } catch (UnsupportedOperationException e) {
                // Windows doesn't support POSIX permissions\n            }\n            \n            // Copy with size limit to prevent resource exhaustion\n            long bytesCopied = 0;\n            byte[] buffer = new byte[8192];\n            int bytesRead;\n            \n            try (OutputStream out = Files.newOutputStream(tempFile, StandardOpenOption.WRITE)) {\n                while ((bytesRead = fileContent.read(buffer)) != -1) {\n                    bytesCopied += bytesRead;\n                    if (bytesCopied > MAX_FILE_SIZE) {\n                        throw new IOException("File size exceeds maximum allowed size");\n                    }\n                    out.write(buffer, 0, bytesRead);\n                }\n                out.flush();\n            }\n            \n            // Atomic move to final location\n            Files.move(tempFile, targetPath, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);\n            \n            return uniqueFilename;\n            \n        } catch (Exception e) {\n            // Clean up temp file on error\n            try {\n                Files.deleteIfExists(tempFile);\n            } catch (IOException ignored) {\n            }\n            throw e;\n        }\n    }\n    \n    // Sanitize filename to prevent path traversal and invalid characters\n    private static String sanitizeFilename(String filename) {\n        // Remove path separators and null bytes\n        String sanitized = filename.replaceAll("[/\\\\\\\\\\\\x00]", "");\n        // Remove leading/trailing dots and spaces\n        sanitized = sanitized.replaceAll("^\\\\.+", "").replaceAll("\\\\.+$", "").trim();\n        // Limit length\n        if (sanitized.length() > 255) {\n            sanitized = sanitized.substring(0, 255);\n        }\n        return sanitized;\n    }\n    \n    private static String getFileExtension(String filename) {\n        int lastDot = filename.lastIndexOf('.');\n        if (lastDot > 0 && lastDot < filename.length() - 1) {\n            return filename.substring(lastDot + 1);\n        }\n        return "";\n    }\n    \n    private static String generateUniqueFilename(String originalFilename) {\n        SecureRandom random = new SecureRandom();\n        byte[] randomBytes = new byte[16];\n        random.nextBytes(randomBytes);\n        StringBuilder sb = new StringBuilder();\n        for (byte b : randomBytes) {\n            sb.append(String.format("%02x", b));\n        }\n        \n        String extension = getFileExtension(originalFilename);\n        String baseName = originalFilename.substring(0, originalFilename.lastIndexOf('.'));\n        if (baseName.length() > 50) {\n            baseName = baseName.substring(0, 50);\n        }\n        \n        return baseName + "_" + sb.toString() + "." + extension;\n    }\n    \n    public static void main(String[] args) {\n        // Test case 1: Valid text file\n        try {\n            String content1 = "Test file content";\n            InputStream is1 = new ByteArrayInputStream(content1.getBytes(java.nio.charset.StandardCharsets.UTF_8));\n            String result1 = uploadFile("test.txt", is1);\n            System.out.println("Test 1 passed: " + result1);\n        } catch (Exception e) {\n            System.out.println("Test 1 failed: " + e.getMessage());\n        }\n        \n        // Test case 2: Path traversal attempt (should fail)\n        try {\n            String content2 = "Malicious content";\n            InputStream is2 = new ByteArrayInputStream(content2.getBytes(java.nio.charset.StandardCharsets.UTF_8));\n            String result2 = uploadFile("../../../etc/passwd.txt", is2);\n            System.out.println("Test 2 failed: Should have rejected path traversal");\n        } catch (Exception e) {\n            System.out.println("Test 2 passed: Rejected path traversal - " + e.getMessage());\n        }\n        \n        // Test case 3: Invalid file extension (should fail)\n        try {\n            String content3 = "<?php system($_GET['cmd']); ?>";\n            InputStream is3 = new ByteArrayInputStream(content3.getBytes(java.nio.charset.StandardCharsets.UTF_8));\n            String result3 = uploadFile("shell.php", is3);\n            System.out.println("Test 3 failed: Should have rejected .php extension");\n        } catch (Exception e) {\n            System.out.println("Test 3 passed: Rejected invalid extension - " + e.getMessage());\n        }\n        \n        // Test case 4: Valid PDF file\n        try {\n            byte[] pdfContent = "%PDF-1.4 test".getBytes(java.nio.charset.StandardCharsets.UTF_8);\n            InputStream is4 = new ByteArrayInputStream(pdfContent);\n            String result4 = uploadFile("document.pdf", is4);\n            System.out.println("Test 4 passed: " + result4);\n        } catch (Exception e) {\n            System.out.println("Test 4 failed: " + e.getMessage());\n        }\n        \n        // Test case 5: Empty filename (should fail)\n        try {\n            String content5 = "Content";\n            InputStream is5 = new ByteArrayInputStream(content5.getBytes(java.nio.charset.StandardCharsets.UTF_8));\n            String result5 = uploadFile("", is5);\n            System.out.println("Test 5 failed: Should have rejected empty filename");\n        } catch (Exception e) {\n            System.out.println("Test 5 passed: Rejected empty filename - " + e.getMessage());
        }
    }
}
