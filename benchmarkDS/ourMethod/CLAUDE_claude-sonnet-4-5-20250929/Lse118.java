
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.security.SecureRandom;
import java.util.Set;

public class Lse118 {
    private static final int MAX_FILE_SIZE = 1024 * 1024; // 1MB limit
    private static final String BASE_DIR = System.getProperty("user.dir");
    
    // Secure random for generating floats
    private static final SecureRandom secureRandom = new SecureRandom();
    
    /**
     * Generates 3 random floats, converts to strings, concatenates them,
     * and writes to a file securely.
     * @param filename The target filename (must be relative path)
     * @return true if successful, false otherwise
     */
    public static boolean generateAndWriteRandomFloats(String filename) {
        if (filename == null || filename.isEmpty()) {
            System.err.println("Invalid filename");
            return false;
        }
        
        // Validate filename - reject path traversal attempts
        if (filename.contains("..") || filename.contains("/") || filename.contains("\\\\")) {\n            System.err.println("Invalid filename: path traversal detected");\n            return false;\n        }\n        \n        try {\n            // Generate 3 random floats using SecureRandom\n            float float1 = secureRandom.nextFloat();\n            float float2 = secureRandom.nextFloat();\n            float float3 = secureRandom.nextFloat();\n            \n            // Convert to strings and concatenate\n            String concatenated = String.valueOf(float1) + String.valueOf(float2) + String.valueOf(float3);\n            \n            // Validate concatenated string size\n            byte[] data = concatenated.getBytes(StandardCharsets.UTF_8);\n            if (data.length > MAX_FILE_SIZE) {\n                System.err.println("Data exceeds maximum file size");\n                return false;\n            }\n            \n            // Resolve path within base directory\n            Path basePath = Paths.get(BASE_DIR).toRealPath();\n            Path targetPath = basePath.resolve(filename).normalize();\n            \n            // Ensure resolved path is within base directory\n            if (!targetPath.startsWith(basePath)) {\n                System.err.println("Path resolves outside base directory");\n                return false;\n            }\n            \n            // Create temp file in same directory with restrictive permissions\n            Path tempFile = Files.createTempFile(basePath, "temp_", ".tmp");\n            \n            try {\n                // Set restrictive permissions (owner read/write only) if POSIX\n                try {\n                    Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");\n                    Files.setPosixFilePermissions(tempFile, perms);\n                } catch (UnsupportedOperationException e) {\n                    // Not a POSIX system, permissions handled by createTempFile\n                }\n                \n                // Write to temp file\n                try (FileOutputStream fos = new FileOutputStream(tempFile.toFile())) {\n                    fos.write(data);\n                    fos.flush();\n                    fos.getFD().sync(); // Ensure data is written to disk\n                }\n                \n                // Atomic move to target location\n                Files.move(tempFile, targetPath, \n                    java.nio.file.StandardCopyOption.REPLACE_EXISTING,\n                    java.nio.file.StandardCopyOption.ATOMIC_MOVE);\n                \n                return true;\n                \n            } catch (Exception e) {\n                // Clean up temp file on error\n                try {\n                    Files.deleteIfExists(tempFile);\n                } catch (IOException cleanupEx) {\n                    // Log cleanup failure but don't mask original error\n                }\n                throw e;\n            }\n            \n        } catch (IOException e) {\n            System.err.println("File operation failed");\n            return false;\n        } catch (Exception e) {\n            System.err.println("Operation failed");\n            return false;\n        }\n    }\n    \n    public static void main(String[] args) {\n        // Test case 1: Valid filename\n        System.out.println("Test 1: " + generateAndWriteRandomFloats("output1.txt"));\n        \n        // Test case 2: Another valid filename\n        System.out.println("Test 2: " + generateAndWriteRandomFloats("output2.txt"));\n        \n        // Test case 3: Valid filename\n        System.out.println("Test 3: " + generateAndWriteRandomFloats("result.txt"));\n        \n        // Test case 4: Invalid - path traversal attempt\n        System.out.println("Test 4: " + generateAndWriteRandomFloats("../output.txt"));\n        \n        // Test case 5: Invalid - null filename\n        System.out.println("Test 5: " + generateAndWriteRandomFloats(null));
    }
}
