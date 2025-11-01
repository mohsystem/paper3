
import java.io.*;
import java.nio.file.*;
import java.nio.charset.StandardCharsets;

public class Lse101 {
    // Security: Use a defined base directory to prevent path traversal
    private static final Path BASE_DIR = Paths.get(System.getProperty("user.home"), "app_data").toAbsolutePath().normalize();
    
    public static void writeConfig(String filename, String content) throws IOException {
        // Security: Validate input parameters
        if (filename == null || filename.trim().isEmpty()) {
            throw new IllegalArgumentException("Filename cannot be null or empty");
        }
        if (content == null) {
            throw new IllegalArgumentException("Content cannot be null");
        }
        
        // Security: Ensure base directory exists with restrictive permissions
        if (!Files.exists(BASE_DIR)) {
            Files.createDirectories(BASE_DIR);
            // Set restrictive permissions (owner read/write/execute only)
            try {
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rwx------");
                Files.setPosixFilePermissions(BASE_DIR, perms);
            } catch (UnsupportedOperationException e) {
                // Windows doesn't support POSIX permissions, use alternative\n                File dir = BASE_DIR.toFile();\n                dir.setReadable(true, true);\n                dir.setWritable(true, true);\n                dir.setExecutable(true, true);\n            }\n        }\n        \n        // Security: Normalize and validate the path to prevent traversal\n        Path targetPath = BASE_DIR.resolve(filename).normalize();\n        if (!targetPath.startsWith(BASE_DIR)) {\n            throw new SecurityException("Path traversal attempt detected");\n        }\n        \n        // Security: Use a temporary file for atomic write operation\n        Path tempFile = Files.createTempFile(BASE_DIR, ".tmp_", "_config");\n        \n        try {\n            // Security: Set restrictive permissions on temp file (owner read/write only)\n            try {\n                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");\n                Files.setPosixFilePermissions(tempFile, perms);\n            } catch (UnsupportedOperationException e) {\n                // Windows fallback\n                File f = tempFile.toFile();\n                f.setReadable(true, true);\n                f.setWritable(true, true);\n                f.setExecutable(false);\n            }\n            \n            // Security: Write using try-with-resources for automatic resource cleanup\n            // Using StandardCharsets.UTF_8 for consistent encoding\n            try (BufferedWriter writer = Files.newBufferedWriter(tempFile, StandardCharsets.UTF_8,\n                    StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING)) {\n                writer.write(content);\n                writer.flush();\n            }\n            \n            // Security: Atomic move to prevent partial writes or race conditions\n            Files.move(tempFile, targetPath, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);\n            \n        } catch (Exception e) {\n            // Security: Clean up temp file on failure\n            try {\n                Files.deleteIfExists(tempFile);\n            } catch (IOException cleanupEx) {\n                // Log but don't mask original exception
            }
            throw e;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal write operation
        try {
            writeConfig("important_config", "important_config");
            System.out.println("Test 1 passed: File written successfully");
        } catch (Exception e) {
            System.err.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: Write with different content
        try {
            writeConfig("important_config", "test_content_123");
            System.out.println("Test 2 passed: File updated successfully");
        } catch (Exception e) {
            System.err.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: Invalid filename (null)
        try {
            writeConfig(null, "content");
            System.err.println("Test 3 failed: Should have thrown exception");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 3 passed: Null filename rejected");
        } catch (Exception e) {
            System.err.println("Test 3 failed with unexpected exception: " + e.getMessage());
        }
        
        // Test case 4: Path traversal attempt
        try {
            writeConfig("../../../etc/passwd", "malicious");
            System.err.println("Test 4 failed: Path traversal not blocked");
        } catch (SecurityException e) {
            System.out.println("Test 4 passed: Path traversal blocked");
        } catch (Exception e) {
            System.err.println("Test 4 failed with unexpected exception: " + e.getMessage());
        }
        
        // Test case 5: Empty content
        try {
            writeConfig("empty_config", "");
            System.out.println("Test 5 passed: Empty content written");
        } catch (Exception e) {
            System.err.println("Test 5 failed: " + e.getMessage());
        }
    }
}
