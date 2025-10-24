
import java.io.*;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.security.SecureRandom;
import java.util.*;

public class Task50 {
    // Security: Maximum file size to prevent DoS and memory exhaustion (CWE-400)
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
    // Security: Maximum filename length to prevent buffer issues (CWE-120)
    private static final int MAX_FILENAME_LEN = 255;
    // Security: Base directory for uploads - prevents path traversal (CWE-22)
    private static final String BASE_UPLOAD_DIR = "./uploads";
    // Security: Buffer size for safe file operations
    private static final int BUFFER_SIZE = 8192;

    /**
     * Security: Validates filename to prevent path traversal attacks (CWE-22, CWE-73)
     * Rejects: "..", "/", "\\\\", null bytes, and control characters\n     * Only allows: safe characters for filenames\n     */\n    private static boolean validateFilename(String filename) {\n        // Security: Null and empty check (CWE-20)\n        if (filename == null || filename.isEmpty()) {\n            System.err.println("Error: Invalid filename");\n            return false;\n        }\n\n        // Security: Length validation (CWE-120, CWE-131)\n        if (filename.length() > MAX_FILENAME_LEN) {\n            System.err.println("Error: Filename too long");\n            return false;\n        }\n\n        // Security: Reject files starting with dangerous characters (CWE-22)\n        char firstChar = filename.charAt(0);\n        if (firstChar == '.' || firstChar == '/' || firstChar == '\\\\') {\n            System.err.println("Error: Filename cannot start with '.', '/', or '\\\\'");\n            return false;\n        }\n\n        // Security: Check for null bytes and path traversal (CWE-158, CWE-22)\n        if (filename.contains("\\0") || filename.contains("..") || \n            filename.contains("/") || filename.contains("\\\\")) {\n            System.err.println("Error: Invalid characters in filename");\n            return false;\n        }\n\n        // Security: Reject control characters (CWE-20)\n        for (char c : filename.toCharArray()) {\n            if (c < 32 || c == 127) {\n                System.err.println("Error: Control characters not allowed");\n                return false;\n            }\n        }\n\n        // Security: Reject dangerous filenames\n        if (filename.equals(".") || filename.equals("..")) {\n            System.err.println("Error: Invalid filename");\n            return false;\n        }\n\n        return true;\n    }\n\n    /**\n     * Security: Constructs and validates safe file path within base directory (CWE-22, CWE-73)\n     * Returns null if path would escape base directory\n     */\n    private static Path constructSafePath(String filename) {\n        // Security: Validate filename first (CWE-20)\n        if (!validateFilename(filename)) {\n            return null;\n        }\n\n        try {\n            // Security: Construct path and normalize it (CWE-22)\n            Path basePath = Paths.get(BASE_UPLOAD_DIR).toRealPath(LinkOption.NOFOLLOW_LINKS);\n            Path targetPath = basePath.resolve(filename).normalize();\n\n            // Security: Ensure target path is within base directory (CWE-22)\n            // This prevents path traversal attacks\n            if (!targetPath.startsWith(basePath)) {\n                System.err.println("Error: Path traversal detected");\n                return null;\n            }\n\n            // Security: Additional check to prevent symlink attacks (CWE-59)\n            if (Files.exists(targetPath, LinkOption.NOFOLLOW_LINKS)) {\n                if (Files.isSymbolicLink(targetPath)) {\n                    System.err.println("Error: Symbolic links not allowed");\n                    return null;\n                }\n            }\n\n            return targetPath;\n\n        } catch (IOException e) {\n            System.err.println("Error: Path construction failed: " + e.getMessage());\n            return null;\n        }\n    }\n\n    /**\n     * Security: Creates upload directory with restrictive permissions (CWE-732)\n     * Uses permissions that allow only owner access\n     */\n    private static boolean ensureUploadDirectory() {\n        try {\n            Path uploadDir = Paths.get(BASE_UPLOAD_DIR);\n\n            if (Files.exists(uploadDir)) {\n                // Security: Verify it's a directory (CWE-22)
                if (!Files.isDirectory(uploadDir)) {
                    System.err.println("Error: Upload path exists but is not a directory");
                    return false;
                }
                return true;
            }

            // Security: Create directory with restrictive permissions (CWE-732)
            // On POSIX systems, set owner-only permissions
            try {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rwx------");
                Files.createDirectory(uploadDir, PosixFilePermissions.asFileAttribute(perms));
            } catch (UnsupportedOperationException e) {
                // On non-POSIX systems (Windows), create directory normally
                Files.createDirectory(uploadDir);
            }

            return true;

        } catch (IOException e) {
            System.err.println("Error: Failed to create upload directory: " + e.getMessage());
            return false;
        }
    }

    /**
     * Security: Race-condition safe file upload using atomic operations (CWE-367, CWE-363)
     * Uses StandardOpenOption.CREATE_NEW to prevent overwriting (CWE-59)
     */
    public static boolean uploadFileSafe(String filename, byte[] data) {
        // Security: Input validation (CWE-20)
        if (filename == null || data == null) {
            System.err.println("Error: Invalid input parameters");
            return false;
        }

        // Security: Validate file size to prevent DoS (CWE-400)
        if (data.length == 0 || data.length > MAX_FILE_SIZE) {
            System.err.println("Error: Invalid file size");
            return false;
        }

        // Security: Ensure upload directory exists with proper permissions
        if (!ensureUploadDirectory()) {
            return false;
        }

        // Security: Construct and validate safe path (CWE-22)
        Path targetPath = constructSafePath(filename);
        if (targetPath == null) {
            return false;
        }

        // Security: Use try-with-resources for automatic resource management (CWE-404, CWE-772)
        try (OutputStream out = Files.newOutputStream(targetPath,
                StandardOpenOption.CREATE_NEW,    // Fails if file exists (CWE-59, CWE-367)
                StandardOpenOption.WRITE,          // Write mode
                StandardOpenOption.SYNC)) {        // Synchronous writes (CWE-662)

            // Security: Write data in a single operation (CWE-252)
            out.write(data);

            // Security: Explicit flush to ensure data is written (CWE-662)
            out.flush();

            System.out.println("Success: File '" + filename + "' uploaded successfully (" + 
                             data.length + " bytes)");
            return true;

        } catch (FileAlreadyExistsException e) {
            System.err.println("Error: File already exists");
            return false;

        } catch (IOException e) {
            System.err.println("Error: File operation failed: " + e.getMessage());
            // Security: Clean up on error (CWE-459)
            try {
                Files.deleteIfExists(targetPath);
            } catch (IOException cleanupEx) {
                // Log but don't fail on cleanup error\n                System.err.println("Warning: Cleanup failed: " + cleanupEx.getMessage());\n            }\n            return false;\n\n        } catch (Exception e) {\n            System.err.println("Error: Unexpected error: " + e.getMessage());\n            // Security: Clean up on error\n            try {\n                Files.deleteIfExists(targetPath);\n            } catch (IOException cleanupEx) {\n                System.err.println("Warning: Cleanup failed: " + cleanupEx.getMessage());\n            }\n            return false;\n        }\n    }\n\n    /**\n     * Main method with 5 test cases\n     */\n    public static void main(String[] args) {\n        System.out.println("=== File Upload Server Test Cases ===\\n");\n\n        // Test Case 1: Normal file upload\n        System.out.println("Test 1: Normal file upload");\n        byte[] testData1 = "This is test file content for test case 1.".getBytes(java.nio.charset.StandardCharsets.UTF_8);\n        if (uploadFileSafe("test1.txt", testData1)) {\n            System.out.println("Test 1: PASSED\\n");\n        } else {\n            System.out.println("Test 1: FAILED\\n");\n        }\n\n        // Test Case 2: Different content file\n        System.out.println("Test 2: Different content file");\n        byte[] testData2 = "Test case 2: Different content with more data to test handling.".getBytes(java.nio.charset.StandardCharsets.UTF_8);\n        if (uploadFileSafe("test2.txt", testData2)) {\n            System.out.println("Test 2: PASSED\\n");\n        } else {\n            System.out.println("Test 2: FAILED\\n");\n        }\n\n        // Test Case 3: Small file\n        System.out.println("Test 3: Small file upload");\n        byte[] testData3 = "Small".getBytes(java.nio.charset.StandardCharsets.UTF_8);\n        if (uploadFileSafe("small_file.txt", testData3)) {\n            System.out.println("Test 3: PASSED\\n");\n        } else {\n            System.out.println("Test 3: FAILED\\n");\n        }\n\n        // Test Case 4: Larger file\n        System.out.println("Test 4: Larger file upload");\n        byte[] largeData = new byte[1024];\n        // Security: Initialize with predictable test data (not for crypto use)\n        for (int i = 0; i < largeData.length; i++) {\n            largeData[i] = (byte) ('A' + (i % 26));
        }
        if (uploadFileSafe("large_test.txt", largeData)) {
            System.out.println("Test 4: PASSED\\n");
        } else {
            System.out.println("Test 4: FAILED\\n");
        }

        // Test Case 5: Security test - path traversal attempt (should fail)
        System.out.println("Test 5: Security test - path traversal attempt (should fail)");
        byte[] testData5 = "Malicious content".getBytes(java.nio.charset.StandardCharsets.UTF_8);
        if (!uploadFileSafe("../etc/passwd", testData5)) {
            System.out.println("Test 5: PASSED (correctly rejected malicious filename)\\n");
        } else {
            System.out.println("Test 5: FAILED (security vulnerability detected!)\\n");
        }

        // Security: Clear sensitive data arrays (best effort)
        Arrays.fill(testData1, (byte) 0);
        Arrays.fill(testData2, (byte) 0);
        Arrays.fill(testData3, (byte) 0);
        Arrays.fill(largeData, (byte) 0);
        Arrays.fill(testData5, (byte) 0);

        System.out.println("=== All tests completed ===");
    }
}
