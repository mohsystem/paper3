
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.nio.channels.FileChannel;
import java.util.Set;

public class Lse103 {
    
    // Secure file creation with O_CREAT|O_WRONLY|O_TRUNC flags
    // Returns FileChannel for writing, caller must close it
    public static FileChannel openFileCreate(String baseDir, String filename) throws IOException {
        // Validate inputs - reject null or empty
        if (baseDir == null || baseDir.isEmpty() || filename == null || filename.isEmpty()) {
            throw new IllegalArgumentException("Base directory and filename must not be null or empty");
        }
        
        // Normalize and validate paths to prevent traversal
        Path basePath = Paths.get(baseDir).toAbsolutePath().normalize();
        Path filePath = basePath.resolve(filename).normalize();
        
        // Security: ensure resolved path is within base directory
        if (!filePath.startsWith(basePath)) {
            throw new SecurityException("Path traversal attempt detected");
        }
        
        // Security: reject if filename contains path separators
        if (filename.contains("/") || filename.contains("\\\\")) {\n            throw new IllegalArgumentException("Filename must not contain path separators");\n        }\n        \n        // Ensure base directory exists\n        if (!Files.exists(basePath)) {\n            throw new IOException("Base directory does not exist");\n        }\n        \n        // O_CREAT|O_WRONLY|O_TRUNC: create or truncate file, write-only\n        // Security: use restrictive permissions (owner read/write only)\n        try {\n            // Set restrictive permissions at creation time (0600)\n            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");\n            \n            FileChannel channel = FileChannel.open(filePath,\n                StandardOpenOption.CREATE,\n                StandardOpenOption.WRITE,\n                StandardOpenOption.TRUNCATE_EXISTING);\n            \n            // Apply restrictive permissions after creation if POSIX supported\n            try {\n                Files.setPosixFilePermissions(filePath, perms);\n            } catch (UnsupportedOperationException e) {\n                // Not a POSIX system, permissions may differ\n            }\n            \n            return channel;\n        } catch (IOException e) {\n            throw new IOException("Failed to open file: " + e.getMessage());\n        }\n    }\n    \n    // Secure file creation with O_CREAT|O_WRONLY|O_EXCL flags\n    // Returns FileChannel for writing, fails if file exists\n    public static FileChannel openFileCreateExclusive(String baseDir, String filename) throws IOException {\n        // Validate inputs\n        if (baseDir == null || baseDir.isEmpty() || filename == null || filename.isEmpty()) {\n            throw new IllegalArgumentException("Base directory and filename must not be null or empty");\n        }\n        \n        // Normalize and validate paths\n        Path basePath = Paths.get(baseDir).toAbsolutePath().normalize();\n        Path filePath = basePath.resolve(filename).normalize();\n        \n        // Security: ensure resolved path is within base directory\n        if (!filePath.startsWith(basePath)) {\n            throw new SecurityException("Path traversal attempt detected");\n        }\n        \n        // Security: reject if filename contains path separators\n        if (filename.contains("/") || filename.contains("\\\\")) {\n            throw new IllegalArgumentException("Filename must not contain path separators");\n        }\n        \n        // Ensure base directory exists\n        if (!Files.exists(basePath)) {\n            throw new IOException("Base directory does not exist");\n        }\n        \n        // O_CREAT|O_WRONLY|O_EXCL: create new file, fail if exists\n        // Security: use restrictive permissions (owner read/write only)\n        try {\n            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");\n            \n            FileChannel channel = FileChannel.open(filePath,\n                StandardOpenOption.CREATE_NEW, // Fails if file exists (O_EXCL behavior)\n                StandardOpenOption.WRITE);\n            \n            // Apply restrictive permissions\n            try {\n                Files.setPosixFilePermissions(filePath, perms);\n            } catch (UnsupportedOperationException e) {\n                // Not a POSIX system\n            }\n            \n            return channel;\n        } catch (IOException e) {\n            throw new IOException("Failed to create file (may already exist): " + e.getMessage());\n        }\n    }\n    \n    public static void main(String[] args) {\n        String testDir = System.getProperty("java.io.tmpdir");\n        \n        // Test case 1: Create or truncate file\n        System.out.println("Test 1: Create/truncate file");\n        try (FileChannel fc = openFileCreate(testDir, "test1.txt")) {\n            System.out.println("Success: File created/truncated");\n        } catch (Exception e) {\n            System.err.println("Error: " + e.getMessage());\n        }\n        \n        // Test case 2: Create exclusive file (should succeed first time)\n        System.out.println("\
Test 2: Create exclusive file");\n        try (FileChannel fc = openFileCreateExclusive(testDir, "test2.txt")) {\n            System.out.println("Success: Exclusive file created");\n        } catch (Exception e) {\n            System.err.println("Error: " + e.getMessage());\n        }\n        \n        // Test case 3: Create exclusive file again (should fail)\n        System.out.println("\
Test 3: Create exclusive file that exists (should fail)");\n        try (FileChannel fc = openFileCreateExclusive(testDir, "test2.txt")) {\n            System.out.println("Success: Exclusive file created");\n        } catch (Exception e) {\n            System.out.println("Expected error: " + e.getMessage());\n        }\n        \n        // Test case 4: Path traversal attempt (should fail)\n        System.out.println("\
Test 4: Path traversal attempt (should fail)");\n        try (FileChannel fc = openFileCreate(testDir, "../etc/passwd")) {\n            System.out.println("Success: File created");\n        } catch (Exception e) {\n            System.out.println("Expected error: " + e.getMessage());\n        }\n        \n        // Test case 5: Invalid input (should fail)\n        System.out.println("\
Test 5: Invalid empty filename (should fail)");\n        try (FileChannel fc = openFileCreate(testDir, "")) {\n            System.out.println("Success: File created");\n        } catch (Exception e) {\n            System.out.println("Expected error: " + e.getMessage());\n        }\n        \n        // Cleanup\n        try {\n            Files.deleteIfExists(Paths.get(testDir, "test1.txt"));\n            Files.deleteIfExists(Paths.get(testDir, "test2.txt"));
        } catch (IOException e) {
            // Ignore cleanup errors
        }
    }
}
